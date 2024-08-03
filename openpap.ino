#include "HX711.h"
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pins for KY-040 rotary encoder
#define CLK_PIN 5
#define DT_PIN 4
#define SW_PIN 18

#define CONFIG_FREERTOS_UNICORE 1 // Add this line for FreeRTOS Unicore
#define CONFIG_FREERTOS_HZ 1000


#define ESC_PIN 15
#define PWM_PIN 15
#define TACH_PIN 2
#define SCK_PIN 16
#define OUT_PIN 17

#define ESC_MIN 900
#define ESC_MAX 2000

#define CALIBRATION 6283.0

#define USE_PWM false
#define USE_ESC true

Servo ESC;  
HX711 scale;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


enum State {
  STATE_INIT,
  STATE_READY,
  STATE_RUNNING,
  STATE_ERROR
};

State state = STATE_INIT;

class PID {
  public:

	double kp;
  double ki;
  double kd;
  double *input;
  double *output; 
  double *setpoint;
	double output_sum, last_input;
	double out_min, out_max;

  PID(double* input, double* output, double* setpoint, double Kp, double Ki, double Kd, double out_min, double out_max) {
    this->output = output;
    this->input = input;
    this->setpoint = setpoint;
    this->out_min = out_min;
    this->out_max = out_max;
    this->kp = Kp;
    this->ki = Ki;
    this->kd = Kd;
    output_sum = *output;
    last_input = *input;
    if(output_sum > out_max) output_sum = out_max;
    else if(output_sum < out_min) output_sum = out_min;
  }

  void compute() {
    double input = *(this->input);
    double error = *setpoint - input;
    double delta_input = (input - last_input);
    output_sum+= (ki * error);
    if(output_sum > out_max) output_sum = out_max;
    else if(output_sum < out_min) output_sum = out_min;
    double output = kp * error;
    output += output_sum - kd * delta_input;
    if(output > out_max) output = out_max;
    else if(output < out_min) output = out_min;
    *(this->output) = output;
  }

};



void esc_setup() {
  display.println("Init ESC...");
  display.display();
  ESC.attach(ESC_PIN);
  ESC.writeMicroseconds(ESC_MIN);
  /*for (int i=ESC_MIN; i<ESC_MIN+200; ++i) {
    Serial.println(i);
    ESC.writeMicroseconds(i);
    delay(20);
  }
  for (int i=ESC_MIN+200; i<ESC_MIN; --i) {
    Serial.println(i);
    ESC.writeMicroseconds(i);
    delay(20);
  }//*/
  delay(500);
}

int tach = 0;
float tach_ma = 0;
float tach_ma_weight = 1.0 / 20.0;
void IRAM_ATTR handle_tach() {
  ++tach;
}

void pwm_setup() {
  attachInterrupt(TACH_PIN, handle_tach, RISING);
  ledcAttach(PWM_PIN, 4000, 9);
  ledcWrite(PWM_PIN, 0);
}

void setup_display() {
  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Display initial text
  display.setCursor(0, 0);
  display.println("Open CPAP / BiPAP");
  display.println("(C) Derek Anderson\n");
  display.display(); // Display text
  delay(1000);
}

void setup_scale() {
  scale.begin(OUT_PIN, SCK_PIN);
  display.println("Taring sensor...");
  display.display();
  Serial.println("Before setting up the scale:");
  Serial.print("read average: ");
  Serial.println(scale.read_average(10));              // print the average of 20 readings from the ADC
  scale.tare(); 
}

bool button_pressed = false;
void IRAM_ATTR handle_button() {
  button_pressed = true;
}


void setup_button() {
  pinMode(SW_PIN, INPUT_PULLDOWN);
  attachInterrupt(SW_PIN, handle_button, HIGH);
}

void cpap(void *params) {
  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t delay_ms = 100 / portTICK_PERIOD_MS;
  double set_point, input, output;
  set_point = 70;
  PID pid(&input, &output, &set_point, 4, 3, 0, USE_ESC ? ESC_MIN : 0, USE_ESC ? ESC_MAX-ESC_MIN : 512);

  while (!button_pressed) {
    int now = millis();
    input = scale.get_value(1) / CALIBRATION;
    pid.compute();
    if (USE_PWM) ledcWrite(PWM_PIN, (int)output);
    if (USE_ESC) ESC.writeMicroseconds(ESC_MIN+(int)output);
    double fan_pct = output*100/(USE_ESC ? ESC_MAX-ESC_MIN : 512);
    Serial.print("pressure:");
    Serial.print(input/10);
    Serial.print(",fan:");
    Serial.print(fan_pct);
    Serial.print(",now:");
    Serial.print(now);
    Serial.println();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Open CPAP / BiPAP\n");
    display.println("Pressure (cm H2O)");
    display.print("");
    display.print(set_point/10);
    display.print(" => ");
    display.println(input/10);
    display.println();
    display.println("Fan (% => RPM)");
    display.print("");
    display.print(fan_pct);
    display.print(" => ");
    display.println(tach_ma/tach_ma_weight*60);
    display.display();
    vTaskDelayUntil(&last_wake_time, delay_ms);
  }
  button_pressed = false;
  wind_down(output);
  ESP.restart();
}


void setup() {
  Serial.begin(115200);

  setup_display();

  if (USE_ESC) esc_setup();
  if (USE_PWM) pwm_setup();

  setup_scale();

  setup_button();

  display.println("Ready!");
  display.display();
  Serial.println("start");
  state = STATE_READY;
  delay(1000);
}

void wind_down(double current_speed) {
      if (USE_PWM) {
        for (int i=(int)current_speed; i>0; --i) {
          ledcWrite(PWM_PIN, i);
          delay(1);
        }
      }
      if (USE_ESC) {
        for (int i=ESC_MIN+(int)current_speed; i>ESC_MIN; --i) {
          ESC.writeMicroseconds(i);
          delay(1);
        }
      }
}

void loop() {
  int start = millis();
  tach_ma = tach*tach_ma_weight + tach_ma*(1-tach_ma_weight);
  tach = 0;
  if (state==STATE_RUNNING) {
    delay(1000);
  } else

  if (state==STATE_READY) {
    if (button_pressed) {
      button_pressed = false;
      state = STATE_RUNNING;
      Serial.println("Starting...");
    xTaskCreatePinnedToCore(cpap, "cpap", 4096, (void *)1, 1, NULL, 1);
    }
    delay(500);
  } else

  if (state==STATE_ERROR) {
    if (button_pressed) {
      button_pressed = false;
      ESP.restart();
    }
  } else {
    delay(1000);
  }

}

