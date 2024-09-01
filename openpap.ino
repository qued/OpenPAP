#include "HX711.h"
#include <ESP32Servo.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#include "WiFi.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define VERSION "0.2.0"

#define OPENPAP_BLE_SERVICE   "ab119f42-66fa-11ef-99b5-2b0355ed36bb"
#define CHARID_VERSION        "7c816628-672a-11ef-a20f-535e28811a27"
#define CHARID_UPGRADE        "ee420c74-680e-11ef-abdb-4777a846df35"
#define CHARID_CPAP           "8387ecb2-551e-4665-83e4-7f0fffd1f850"
#define CHARID_IPAP           "1da0bb42-66fb-11ef-9518-3bea9dfbdfe7"
#define CHARID_EPAP           "1e0ee004-66fb-11ef-94c7-571be3538e36"
#define CHARID_KP             "998332a0-67c6-11ef-b2df-e78aa6930f15"
#define CHARID_KI             "9ee75a0a-67c6-11ef-adef-b367f4c7b993"
#define CHARID_KD             "9f4ad8b4-67c6-11ef-a327-57bd08d0ce3f"


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

    double Kp;
    double Ki;
    double Kd;
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
      this->Kp = Kp;
      this->Ki = Ki;
      this->Kd = Kd;
      output_sum = *output;
      last_input = *input;
      if(output_sum > out_max) output_sum = out_max;
      else if(output_sum < out_min) output_sum = out_min;
    }

    void compute() {
      double input = *(this->input);
      double error = *setpoint - input;
      double delta_input = (input - last_input);
      output_sum+= (Ki * error);
      if(output_sum > out_max) output_sum = out_max;
      else if(output_sum < out_min) output_sum = out_min;
      double output = Kp * error;
      output += output_sum - Kd * delta_input;
      if(output > out_max) output = out_max;
      else if(output < out_min) output = out_min;
      *(this->output) = output;
    }

};

double pressure = 1.0; // cmH2O
double Kp = 0.4;
double Ki = 0.01;
double Kd = 0.0;

void load_double(char* name, double* v) {
  if (preferences.isKey(name)) {
    *v = preferences.getFloat(name);
    Serial.printf("Loaded %s: %f\n", name, *v);
  } else {
    Serial.printf("No %s pressure saved, using default: %f\n", name, *v);
  }
}

void setup_preferences() {
  preferences.begin("OpenPAP", false);
  load_double("CPAP", &pressure);
  load_double("Kp", &Kp);
  load_double("Ki", &Ki);
  load_double("Kd", &Kd);
}

class CharDoubleCallback: public BLECharacteristicCallbacks {
  char* name;
  double* value_ptr;
  public: 
  CharDoubleCallback(char* name, double* value_ptr) {
    this->value_ptr = value_ptr;
    this->name = name;
  }
  void onWrite(BLECharacteristic *pCharacteristic) {
    *value_ptr = std::__cxx11::stof(pCharacteristic->getValue().c_str());
    preferences.putFloat(name, *value_ptr);
    Serial.printf("BLE set %s: %f cmH20\n", name, *value_ptr);
  }
};

bool do_upgrade = false;
char ssid[100] = "";
char password[100] = "";

class CharUpgradeCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String str = pCharacteristic->getValue();
    Serial.printf("Got json: %s %s %s\n", str, ssid, password);
    if (str.charAt(0)=='s') strncpy(ssid, str.c_str()+1, sizeof(ssid) - 1);
    if (str.charAt(0)=='p') strncpy(password, str.c_str()+1, sizeof(ssid) - 1);
    if (str.charAt(0)=='u') do_upgrade = true;
  }
};

BLECharacteristic* char_upgrade;

void update_started() {
  Serial.println("CALLBACK:  HTTP update process started");
  char* upgrade_status = "0";
  char_upgrade->setValue((uint8_t*)upgrade_status, strlen(upgrade_status));
}

void update_finished() {
  Serial.println("CALLBACK:  HTTP update process finished");
  delay(10000);
  ESP.restart();
}

void update_progress(int cur, int total) {
  Serial.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
  char upgrade_status[4];
  itoa(100*cur/total, upgrade_status, 10);
  char_upgrade->setValue((uint8_t*)upgrade_status, strlen(upgrade_status));
}

void update_error(int err) {
  Serial.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
  char upgrade_status[20] = "ERROR:";
  itoa(err, upgrade_status+strlen(upgrade_status), 10);
  char_upgrade->setValue((uint8_t*)upgrade_status, strlen(upgrade_status));
}

void run_upgrade() {
  if (!do_upgrade) return;
  Serial.println("Starting OTA...");
  const char* url = "http://openpap.org/arduino/OpenPAP.ino.bin"; //upgrade["url"];
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  NetworkClient client;

  // The line below is optional. It can be used to blink the LED on the board during flashing
  // The LED will be on during download of one buffer of data from the network. The LED will
  // be off during writing that buffer to flash
  // On a good connection the LED should flash regularly. On a bad connection the LED will be
  // on much longer than it will be off. Other pins than LED_BUILTIN may be used. The second
  // value is used to put the LED on. If the LED is on with HIGH, that value should be passed
  //httpUpdate.setLedPin(LED_BUILTIN, LOW);

  httpUpdate.onStart(update_started);
  httpUpdate.onEnd(update_finished);
  httpUpdate.onProgress(update_progress);
  httpUpdate.onError(update_error);

  t_httpUpdate_return ret = httpUpdate.update(client, url);

  switch (ret) {
    case HTTP_UPDATE_FAILED: Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str()); break;

    case HTTP_UPDATE_NO_UPDATES: Serial.println("HTTP_UPDATE_NO_UPDATES"); break;

    case HTTP_UPDATE_OK: Serial.println("HTTP_UPDATE_OK"); break;
  }
}

char device_name[100];

void ble_setup() {
  snprintf(device_name, 100, "OpenPAP - %llX", ESP.getEfuseMac());
  BLEDevice::init(device_name);
  server = BLEDevice::createServer();
  service = server->createService(OPENPAP_BLE_SERVICE);

  BLECharacteristic* char_version = service->createCharacteristic(CHARID_VERSION, BLECharacteristic::PROPERTY_READ);
  char_version->setValue((uint8_t*)VERSION, strlen(VERSION));

  BLECharacteristic* char_cpap = service->createCharacteristic(CHARID_CPAP, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  std::string pressure_s = std::to_string(pressure);
  char_cpap->setValue((uint8_t*)pressure_s.c_str(), pressure_s.length());
  char_cpap->setCallbacks(new CharDoubleCallback("CPAP", &pressure));

  BLECharacteristic* char_Kp = service->createCharacteristic(CHARID_KP, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  std::string Kp_s = std::to_string(Kp);
  char_Kp->setValue((uint8_t*)Kp_s.c_str(), Kp_s.length());
  char_Kp->setCallbacks(new CharDoubleCallback("Kp", &Kp));

  BLECharacteristic* char_Ki = service->createCharacteristic(CHARID_KI, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  std::string Ki_s = std::to_string(Ki);
  char_Ki->setValue((uint8_t*)Ki_s.c_str(), Ki_s.length());
  char_Ki->setCallbacks(new CharDoubleCallback("Ki", &Ki));

  BLECharacteristic* char_Kd = service->createCharacteristic(CHARID_KD, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  std::string Kd_s = std::to_string(Kd);
  char_Kd->setValue((uint8_t*)Kd_s.c_str(), Kd_s.length());
  char_Kd->setCallbacks(new CharDoubleCallback("Kd", &Kd));

  char_upgrade = service->createCharacteristic(CHARID_UPGRADE, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  std::string upgrade_status = "READY";
  char_upgrade->setValue((uint8_t*)upgrade_status.c_str(), upgrade_status.length());
  char_upgrade->setCallbacks(new CharUpgradeCallback());

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
  PID pid(&input, &output, &pressure, Kp, Ki, Kd, 0, USE_ESC ? ESC_MAX-ESC_MIN : 512);

  while (!button_pressed) {
    int now = millis();
    input = scale.get_value(1) / CALIBRATION;
    pid.compute();
    if (USE_PWM) ledcWrite(PWM_PIN, (int)output);
    if (USE_ESC) ESC.writeMicroseconds(ESC_MIN+(int)output);
    double fan_pct = output*100/(USE_ESC ? ESC_MAX-ESC_MIN : 512);
    Serial.print("pressure:");
    Serial.print(input);
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
    display.print(pressure);
    display.print(" => ");
    display.println(input);
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
  display.print(pressure);
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
  run_upgrade();

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

