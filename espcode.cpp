#include <WiFi.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "esp_adc_cal.h"
#include <time.h>

TFT_eSPI tft = TFT_eSPI();

// ==== Thermistor Pins ====
#define PIN_WET 36
#define PIN_DRY 39
#define PIN_BAT 34
#define DISP_PWR_PIN 4

// ==== WiFi Credentials ====
const char* ssid = "wifi-name";
const char* password = "wifi-password";

// ==== n8n Webhook URL ====
const char* webhook_url = "n8nWebhookurl";

// ==== Thermistor constants (your calibration) ====
const float SERIES_RESISTOR = 10000.0f;
const float NOMINAL_RESISTANCE_DRY = 10450.0f;
const float NOMINAL_TEMPERATURE_DRY = 25.0f;
const float BETA_COEFFICIENT_DRY = 3950.0f;

const float NOMINAL_RESISTANCE_WET = 11100.0f;
const float NOMINAL_TEMPERATURE_WET = 25.0f;
const float BETA_COEFFICIENT_WET = 3950.0f;

const uint32_t ADC_SAMPLES = 40;
static esp_adc_cal_characteristics_t adc_chars;

// ==== Timers ====
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 5000;

// ==== ADC Setup ====
void setupADC() {
  analogSetPinAttenuation(PIN_WET, ADC_11db);
  analogSetPinAttenuation(PIN_DRY, ADC_11db);
  analogSetPinAttenuation(PIN_BAT, ADC_11db);
  analogSetWidth(12);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
}

uint32_t readADCmillivolt(int pin) {
  uint32_t sum = 0;
  for (uint32_t i = 0; i < ADC_SAMPLES; ++i) {
    sum += analogRead(pin);
    delay(2);
  }
  uint32_t rawAvg = sum / ADC_SAMPLES;
  return esp_adc_cal_raw_to_voltage(rawAvg, &adc_chars);
}

float calcR_from_Vout_mV(float Vout_mV) {
  const float Vcc_mV = 3300.0f;
  if (Vout_mV <= 0.5f) return 1e9;
  if (Vout_mV >= Vcc_mV - 0.5f) return 1e9;
  return SERIES_RESISTOR * (Vout_mV / (Vcc_mV - Vout_mV));
}

float calcTempC_from_R(float R, float nominalR, float nominalT, float beta) {
  if (R <= 0) return -273.15;
  float steinhart = R / nominalR;
  steinhart = log(steinhart);
  steinhart /= beta;
  steinhart += 1.0f / (nominalT + 273.15f);
  steinhart = 1.0f / steinhart;
  steinhart -= 273.15f;
  return steinhart;
}

// ==== Get Time for JSON ====
String nowIsoMalaysia() {
  time_t rawtime;
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buf[40];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S%z", &timeinfo);
    String s = String(buf);
    if (s.length() >= 5) s = s.substring(0, s.length()-2) + ":" + s.substring(s.length()-2);
    return s;
  } else {
    unsigned long secs = millis() / 1000;
    char buf[32];
    snprintf(buf, sizeof(buf), "1970-01-01T00:00:%02lu+08:00", secs % 60);
    return String(buf);
  }
}

// ==== WiFi Setup ====
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 40);
  tft.println("Connecting WiFi...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) delay(300);

  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2);
  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(TFT_GREEN);
    tft.println("WiFi Connected");
    configTime(8 * 3600, 0, "pool.ntp.org", "time.google.com");
  } else {
    tft.setTextColor(TFT_RED);
    tft.println("WiFi Failed");
  }
  delay(1500);
}

// ==== Send Data to n8n ====
void sendToN8N(float dryTemp, float wetTemp, float batteryV) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(webhook_url);
  http.addHeader("Content-Type", "application/json");

  String timestamp = nowIsoMalaysia();
  String payload = "{";
  payload += "\"device_id\":\"iot-psychrometer\",";
  payload += "\"timestamp\":\"" + timestamp + "\",";
  payload += "\"dry_temp\":" + String(dryTemp, 2) + ",";
  payload += "\"wet_temp\":" + String(wetTemp, 2) + ",";
  payload += "\"battery\":" + String(batteryV, 2);
  payload += "}";

  int code = http.POST(payload);
  Serial.printf("POST code=%d\n", code);
  if (code > 0) {
    Serial.println("Data sent to n8n: " + payload);
  } else {
    Serial.println("Failed to send data");
  }
  http.end();
}

// ==== Setup ====
void setup() {
  Serial.begin(115200);
  pinMode(DISP_PWR_PIN, OUTPUT);
  digitalWrite(DISP_PWR_PIN, HIGH);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  setupADC();
  connectWiFi();

  tft.setTextFont(4);
  tft.setTextColor(TFT_YELLOW);
  tft.setCursor(20, 10);
  tft.println("IOT PSYCHROMETER");
}

// ==== Main Loop ====
void loop() {
  // === Read Sensors ===
  uint32_t wet_mV = readADCmillivolt(PIN_WET);
  uint32_t dry_mV = readADCmillivolt(PIN_DRY);
  uint32_t bat_mV = readADCmillivolt(PIN_BAT);

  float R_wet = calcR_from_Vout_mV((float)wet_mV);
  float R_dry = calcR_from_Vout_mV((float)dry_mV);
  float T_wet = calcTempC_from_R(R_wet, NOMINAL_RESISTANCE_WET, NOMINAL_TEMPERATURE_WET, BETA_COEFFICIENT_WET);
  float T_dry = calcTempC_from_R(R_dry, NOMINAL_RESISTANCE_DRY, NOMINAL_TEMPERATURE_DRY, BETA_COEFFICIENT_DRY);
  float battery_V = (bat_mV / 1000.0f) * 2.0f;

  // === Print to Serial ===
  Serial.printf("WET: %.2f°C | DRY: %.2f°C | BAT: %.2fV\n", T_wet, T_dry, battery_V);

  // === Display on TFT ===
  tft.fillRect(0, 60, 240, 80, TFT_BLACK);
  tft.setTextFont(2);
  tft.setTextColor(TFT_CYAN);
  tft.setCursor(30, 60);
  tft.printf("WET : %.1f °C", T_wet);

  tft.setTextColor(TFT_GREEN);
  tft.setCursor(30, 85);
  tft.printf("DRY : %.1f °C", T_dry);

  tft.setTextColor(TFT_WHITE);
  tft.setCursor(30, 110);
  tft.printf("BAT : %.2f V", battery_V);

  // === Send data every few seconds ===
  if (millis() - lastSendTime > sendInterval) {
    sendToN8N(T_dry, T_wet, battery_V);
    lastSendTime = millis();
  }

  delay(1000);
}
