#include "HX711.h"
#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#include <ArduinoOTA.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define VERSION "0.1.0"

#define OPENPAP_BLE_SERVICE   "ab119f42-66fa-11ef-99b5-2b0355ed36bb"
#define CHARID_VERSION        "7c816628-672a-11ef-a20f-535e28811a27"
#define CHARID_CPAP           "8387ecb2-551e-4665-83e4-7f0fffd1f850"
#define CHARID_IPAP           "1da0bb42-66fb-11ef-9518-3bea9dfbdfe7"
#define CHARID_EPAP           "1e0ee004-66fb-11ef-94c7-571be3538e36"


BLEServer *server = NULL;
BLEService *service = NULL;
BLEAdvertising *advertising = NULL;

Preferences preferences;


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

#define ESC_MIN 1000
#define ESC_MAX 2000

#define CALIBRATION 6283.0

#define USE_PWM false
#define USE_ESC true

#define NAME "OpenPAP"
#define TITLE " --==[ OpenPAP ]==--"
#define COPY "(C) Derek Anderson"
#define LINK " https://openpap.org"

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

  private:

    double kp;
    double ki;
    double kd;
    double *input;
    double *output; 
    double *setpoint;
    double output_sum, last_input;
    double out_min, out_max;

  public:

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

double pressure = 10; // mmH2O

void setup_preferences() {
  preferences.begin("OpenPAP", false);
  if (preferences.isKey("CPAP")) {
    pressure = preferences.getFloat("CPAP");
    Serial.printf("Loaded CPAP pressure: %f mmH20\n", pressure);
  } else {
    Serial.printf("No CPAP pressure saved, using default: %f mmH20\n", pressure);
  }
}

class CharCPAPCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    pressure = std::__cxx11::stof(pCharacteristic->getValue().c_str()) * 10;
    preferences.putFloat("CPAP", pressure);
    Serial.printf("BLE set CPAP: %f mmH20\n", pressure);
  }
};

char device_name[100];

void ble_setup() {
  snprintf(device_name, 100, "OpenPAP - %llX", ESP.getEfuseMac());
  BLEDevice::init(device_name);
  server = BLEDevice::createServer();
  service = server->createService(OPENPAP_BLE_SERVICE);
  BLECharacteristic* char_version = service->createCharacteristic(CHARID_VERSION, BLECharacteristic::PROPERTY_READ);
  char_version->setValue((uint8_t*)VERSION, strlen(VERSION));
  BLECharacteristic* char_cpap = service->createCharacteristic(CHARID_CPAP, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  std::string pressure_s = std::to_string(pressure/10);
  char_cpap->setValue((uint8_t*)pressure_s.c_str(), pressure_s.length());
  char_cpap->setCallbacks(new CharCPAPCallback());

  service->start();
  advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(OPENPAP_BLE_SERVICE);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  advertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("BLE server started.");

}

void esc_setup() {
  display.println("Init ESC...");
  display.display();
  ESC.attach(ESC_PIN);
  ESC.writeMicroseconds(ESC_MIN);
  /*for (int i=ESC_MIN; i<ESC_MIN+300; ++i) {
    Serial.println(i);
    ESC.writeMicroseconds(i);
    delay(20);
  }
  for (int i=ESC_MIN+300; i<ESC_MIN; --i) {
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

  display.setRotation(2);

  // Clear the display buffer
  display.clearDisplay();

  // Set text size and color
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Display initial text
  display.setCursor(0, 0);
  display.println(TITLE);
  display.println(LINK);
  display.println();
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
  Serial.println("Starting...");
  TickType_t last_wake_time = xTaskGetTickCount();
  const TickType_t delay_ms = 100 / portTICK_PERIOD_MS;
  double input, output;
  PID pid(&input, &output, &pressure, .1, .1, 0, 0, USE_ESC ? ESC_MAX-ESC_MIN : 512);

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
    display.println(TITLE);
    display.println("Pressure (cm H2O)");
    display.print("");
    display.print(pressure/10);
    display.print(" => ");
    display.println(input/10);
    display.println();
    display.println("Fan (%)");
    display.print("");
    display.print(fan_pct);
    display.display();
    vTaskDelayUntil(&last_wake_time, delay_ms);
  }
  button_pressed = false;
  wind_down(output);
  ESP.restart();
}

void setup_ota() {
  Serial.println("Setting up OTA...");
  //ArduinoOTA.setPassword("admin");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_SPIFFS
        type = "filesystem";
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });

  ArduinoOTA.begin();
}

void setup() {
  esp_log_level_set("*", ESP_LOG_DEBUG);
  Serial.begin(115200);

  setup_preferences();
  
  setup_display();

  ble_setup();

  if (USE_ESC) esc_setup();
  if (USE_PWM) pwm_setup();

  setup_scale();

  setup_button();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(TITLE);
  display.println(LINK);
  display.println();
  display.print("CPAP: ");
  display.print(pressure/10);
  display.println(" cm H2O");
  display.println("Press to start...");
  display.display();
  Serial.println("READY");
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
  ArduinoOTA.handle();

  if (state==STATE_RUNNING) {
    delay(1000);
  } else
  if (state==STATE_READY) {
    if (button_pressed) {
      button_pressed = false;
      state = STATE_RUNNING;
    xTaskCreatePinnedToCore(cpap, "cpap", 4096, (void *)1, 1, NULL, 1);
    }
    delay(1000);
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

