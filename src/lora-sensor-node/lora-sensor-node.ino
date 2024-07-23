#include <Arduino.h>
#include <SCD30.h>
#include <HardwareSerial.h>
#include <config.ino>

//Defining Hardware the second internal UART - Serial2 for the LoRaWAN E5 Module- Pin 9 and 10.
HardwareSerial Serial2(1);

int soilSensor = A1;
int soilSensorValue = 0;
int soilMoisturePercent = 0;

int batterySensor = A2;
int batterySensorValue = 0;


float temp = 0;
float humi = 0;
float co2 = 0;
float battery = 0;

static char recv_buf[512];
static bool is_exist = false;
static bool is_join = false;
static int led = 0;

void setup() {

  Serial.begin(115200);
  Wire.begin();

  scd30.initialize();
  scd30.setAutoSelfCalibration(1);
  // And configure Serial2 on pins RX=D9, TX=D10
  Serial2.begin(9600, SERIAL_8N1, TX_2, RX_2);
  Serial2.print("Serial2");

  Serial2.begin(9600);

  if (at_send_check_response("+AT: OK", 100, "AT\r\n")) {
    is_exist = true;
    at_send_check_response("+ID: AppEui", 1000, "AT+ID\r\n");
    at_send_check_response("+MODE: LWOTAA", 1000, "AT+MODE=LWOTAA\r\n");
    at_send_check_response("+DR: EU868", 1000, "AT+DR=EU868\r\n");
    at_send_check_response("+CH: NUM", 1000, "AT+CH=NUM,0-2\r\n");
    at_send_check_response("+KEY: APPKEY", 1000, "AT+KEY=APPKEY,\"C2XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\"\r\n");  // Enter your generated APP Key here. [This is generated on add device page, at Things Network Platfrom]
    at_send_check_response("+CLASS: C", 1000, "AT+CLASS=A\r\n");
    at_send_check_response("+PORT: 8", 1000, "AT+PORT=8\r\n");
    delay(200);
    Serial.println("LoRaWAN");
    is_join = true;
  } else {
    is_exist = false;
    Serial.print("No LoRa E5 module found.\r\n");
  }
}

void loop() {

  // When runing deep sleep mode we dont need timers to execute the command as the controller will wake up after 10 mins to excute the code.
  getSensors();
  sendData();
  // Configure the wake-up source and duration for deep sleep
  esp_sleep_enable_timer_wakeup(10 * 60 * 1000000); // 10 minutes in microseconds
  // Enter deep sleep mode
  esp_deep_sleep_start();
}
