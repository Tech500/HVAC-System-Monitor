/*  BME280_Outside_Node.ino
 September 05, 2026 @ 05:33 EDT
  ESP32 Core 3.3.10 Required!!! Earlier breaks compile!
*/

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#define EoRa_PI_V1
#include <boards.h>
#include "SX1262_commands.h"
#include <WiFi.h>
#include <ESP32_NOW.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <BME280I2C.h>
#include <esp_sleep.h>
#include "driver/rtc_io.h"
#include "rom/rtc.h"
#include "esp_system.h"
#include "esp_private/periph_ctrl.h"

#define WAKEUP_PIN GPIO_NUM_16

#define BOARD_LED 37
#define LED_ON HIGH
#define LED_OFF LOW

#define BME_SDA_PIN 47
#define BME_SCL_PIN 48
#define BME_I2C_ADDR 0x76

#define RXDC_RX_TICKS       512UL     
#define RXDC_SLEEP_TICKS 	323648UL    

uint8_t hubMAC[] = { 0x1C, 0xDB, 0xD4, 0x85, 0x6E, 0x9C };

uint8_t HUB_WIFI_CHANNEL = 11;
uint8_t CHANNEL = 0;

#define BME_SDA 47
#define BME_SCL 48

// Flag stored in low-power RTC SRAM (survives ESP32 deep sleep)
RTC_DATA_ATTR bool sxConfigured = false;
RTC_DATA_ATTR uint32_t wakeCount = 0;

const float BME280_OUTSIDE_TEMP_CAL_OFFSET_F = +5.54;
const float STATION_ELEVATION_FT = 791.0f;
const float STATION_ELEVATION_M = STATION_ELEVATION_FT * 0.3048f;

BME280I2C bme;

// ─── Struct Definitions ───
enum MessageType : uint8_t {
  MSG_BME280 = 0,
  MSG_ALERT_FLAG = 1,
  MSG_BLOWER_STATE = 2
};

struct __attribute__((packed)) AlertFlagPacket {
  MessageType type;
  bool alert;
};

struct __attribute__((packed)) BlowerData {
  MessageType type;
  bool on;
  float elapsedMinutes;
  float dailyTotalMinutes;
};

struct __attribute__((packed)) BME280Data {
  MessageType type;
  float temperature;
  float humidity;
  float pressure;
};

// ─── 3. Receiver Global Tracking Registers ──────────────────────────────────
bool alertFlag = false;          // Global state updated by MSG_ALERT_FLAG
bool blowerIsOn = false;         // Global state updated by MSG_BLOWER_STATE
float globalTemp = NAN;          // Global state updated by MSG_BME280
float globalHumidity = NAN;
float globalPressure = NAN;

class HubPeer : public ESP_NOW_Peer {
public:
  HubPeer(const uint8_t *hubMAC, uint8_t CHANNEL)
    : ESP_NOW_Peer(hubMAC, CHANNEL, WIFI_IF_STA, NULL) {}

  bool add_to_system() {
    return ESP_NOW_Peer::add();
  }
  bool remove_from_system() {
    return ESP_NOW_Peer::remove();
  }
  bool sendData(const uint8_t *data, size_t len) {
    return send(data, len);
  }
};

// -------------------------------------------------------------
// TRANSMIT ALERT FLAG TO HUB VIA ESP-NOW
// -------------------------------------------------------------
bool sendAlertFlagToReceiver(bool alertFlag) {
  // 1. Ensure STA mode is active
  WiFi.mode(WIFI_STA);

  // 2. Fetch or assign a valid primary channel (1-14)
  uint8_t primaryChannel = WiFi.channel();
  if (primaryChannel == 0) primaryChannel = HUB_WIFI_CHANNEL > 0 ? HUB_WIFI_CHANNEL : 1;
  
  esp_wifi_set_channel(primaryChannel, WIFI_SECOND_CHAN_NONE);

  if (!ESP_NOW.begin()) {
    Serial.println(F("[ESP-NOW] Engine start failed"));
    return false;
  }

  // 3. Create peer instance on valid primary channel
  HubPeer localHub(hubMAC, primaryChannel);

  // 4. Force-purge any lingering table registration for this MAC
  localHub.remove_from_system();

  // 5. Register peer
  if (!localHub.add_to_system()) {
    Serial.printf("[ESP-NOW Error] Peer add failed on Ch %d. Check hubMAC array.\n", primaryChannel);
    ESP_NOW.end();
    return false;
  }

  // 6. Send payload
  AlertFlagPacket alertPacket = {};
  alertPacket.type = MSG_ALERT_FLAG;
  alertPacket.alert = alertFlag;

  bool result = localHub.sendData((uint8_t *)&alertPacket, sizeof(AlertFlagPacket));

  if (result) {
    Serial.printf("[ESP-NOW] Sent alertFlag: %s\n", alertFlag ? "TRUE" : "FALSE");
  } else {
    Serial.println(F("[ESP-NOW] Transmit failed"));
  }

  // 7. Cleanup
  localHub.remove_from_system();
  ESP_NOW.end();

  return result;
}

float relativePressure(float stationPressureHPa, float elevationMeters) {
  return stationPressureHPa / pow(1.0f - (elevationMeters / 44330.0f), 5.255f);
}

bool readAndSendBME280() {
  Wire.end();
  delay(50);
  Wire.setPins(BME_SDA, BME_SCL);
  if (!Wire.begin(BME_SDA, BME_SCL)) {
    Serial.println(F("Failed to allocate I2C peripheral instance!"));
  }
  delay(50);

  if (!bme.begin()) {
    Serial.println(F("BME280 not found -- check wiring/address"));
    return false;
  }

  float tempF = NAN, humidity = NAN, pressureHPa = NAN;
  BME280::TempUnit tempUnit(BME280::TempUnit_Fahrenheit);
  BME280::PresUnit presUnit(BME280::PresUnit_hPa);

  delay(500);

  bme.read(pressureHPa, tempF, humidity, tempUnit, presUnit);
  tempF += BME280_OUTSIDE_TEMP_CAL_OFFSET_F;

  delay(200);

  if (isnan(tempF) || isnan(pressureHPa)) {
    Serial.println(F("Error reading BME280 telemetry."));
    return false;
  }

  float relPressureHPa = relativePressure(pressureHPa, STATION_ELEVATION_M);

  Serial.printf("BME280 -> Temp: %.2f F  Hum: %.2f %%  Pres: %.4f hPa\n",
                tempF, humidity, relPressureHPa);

  return sendTelemetryViaESPNOW(tempF, humidity, relPressureHPa);
}

bool sendTelemetryViaESPNOW(float tempF, float humidity, float pressureHPa) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_channel(HUB_WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (!ESP_NOW.begin()) {
    Serial.println(F("ESP-NOW init failed"));
    WiFi.mode(WIFI_OFF);
    return false;
  }

  HubPeer localHub(hubMAC, HUB_WIFI_CHANNEL);
  if (!localHub.add_to_system()) {
    Serial.println(F("Failed to bind hub peer"));
    ESP_NOW.end();
    WiFi.mode(WIFI_OFF);
    return false;
  }

  BME280Data pkt;
  pkt.type = MSG_BME280;
  pkt.temperature = tempF;
  pkt.humidity = humidity;
  pkt.pressure = pressureHPa;

  bool sent = localHub.sendData((uint8_t *)&pkt, sizeof(BME280Data));
  Serial.printf("[ESP-NOW] Send to hub: %s\n", sent ? "OK" : "FAILED");

  localHub.remove_from_system();
 
  return sent;
}

#define BENCH_TESTING 1

void preparePinsForSleep() {
  digitalWrite(RADIO_CS_PIN, HIGH);
  gpio_hold_en((gpio_num_t)RADIO_CS_PIN);

  pinMode(RADIO_RST_PIN, INPUT);
  gpio_hold_en((gpio_num_t)RADIO_RST_PIN);

#if !BENCH_TESTING
  Serial.flush();
  periph_module_disable(PERIPH_USB_MODULE);

  pinMode(GPIO_NUM_19, INPUT);
  pinMode(GPIO_NUM_20, INPUT);
  gpio_pullup_dis(GPIO_NUM_19);
  gpio_pulldown_dis(GPIO_NUM_19);
  gpio_pullup_dis(GPIO_NUM_20);
  gpio_pulldown_dis(GPIO_NUM_20);
  gpio_hold_en(GPIO_NUM_19);
  gpio_hold_en(GPIO_NUM_20);
#endif

  gpio_deep_sleep_hold_en();
}

void releasePinHoldsOnWake() {
  gpio_deep_sleep_hold_dis();

  gpio_hold_dis((gpio_num_t)RADIO_CS_PIN);
  gpio_hold_dis((gpio_num_t)RADIO_RST_PIN);

#if !BENCH_TESTING
  gpio_hold_dis(GPIO_NUM_19);
  gpio_hold_dis(GPIO_NUM_20);
#endif

  pinMode(RADIO_RST_PIN, OUTPUT);
  digitalWrite(RADIO_RST_PIN, HIGH);
}

void enterLowPowerWOR() {
  // 1. Standby RC mode to accept duty cycle command
  sxStandby();
  sxWaitBusy();

  // 2. Start Duty Cycle mode (~32ms RX / ~80ms Sleep)
  sxSetRxDutyCycle(RXDC_RX_TICKS, RXDC_SLEEP_TICKS);

  // 3. Clear transient IRQs
  delayMicroseconds(200);
  sxClearIrq();

  // 4. Configure GPIO 16 for RTC EXT0 wake
  gpio_deep_sleep_hold_dis();
  gpio_hold_dis(GPIO_NUM_16);

  rtc_gpio_init(GPIO_NUM_16);
  rtc_gpio_set_direction(GPIO_NUM_16, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pulldown_en(GPIO_NUM_16);

  // 5. Arm EXT0 wake on HIGH (Level 1)
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_16, 1);

  Serial.println("Entering Deep Sleep with RxDutyCycle armed...");

  Serial.flush();

  // 6. Enter Deep Sleep
  esp_deep_sleep_start();
}

void setup() {

  bool isFullBoot = false;

  // 1. Unhold pin and disable sleep triggers
  rtc_gpio_hold_dis(GPIO_NUM_16);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  Serial.begin(115200);
  delay(1500);

  Serial.print("\n\n\nHVAC System Monitor - BME280_Outside_Node + LoRa WOR\n");
  Serial.println("with SX1262 rxDutyCycle\n\n");

  // 2. Initialize SPI hardware pins BEFORE reading registers
  pinMode((gpio_num_t)RADIO_CS_PIN, OUTPUT);
  digitalWrite((gpio_num_t)RADIO_CS_PIN, HIGH);
  pinMode((gpio_num_t)RADIO_BUSY_PIN, INPUT);

  radioSPI.begin(RADIO_SCLK_PIN, RADIO_MISO_PIN, RADIO_MOSI_PIN, RADIO_CS_PIN);

  pinMode(WAKEUP_PIN, INPUT);
  pinMode(BOARD_LED, OUTPUT);

  // 1. Read ESP32 reset reason and sleep wake cause
  //esp_reset_reason_t resetReason = esp_reset_reason();
  esp_sleep_wakeup_cause_t wakeCause = esp_sleep_get_wakeup_cause();

  // Group all cold boot, hardware, and soft-upload reset vectors

  //if (resetReason == (ESP_RST_POWERON || ESP_RST_EXT || ESP_RST_SW || ESP_RST_PANIC)) {
  // isFullBoot = true;
  //}

  // -------------------------------------------------------------
  // 1. FAST PATH: EXT0 DEEP SLEEP WAKE (GPIO 16 / DIO1)
  // -------------------------------------------------------------
  if (wakeCause == ESP_SLEEP_WAKEUP_EXT0) {
    uint32_t rtcPinState = rtc_gpio_get_level(GPIO_NUM_16);
    Serial.printf("[WAKE] EXT0 Triggered by GPIO 16 (RTC Level: %d)\n", rtcPinState);

    sxWakeupSPI();
    sxWaitBusy();

    uint16_t irqStatus = sxGetIrq();
    Serial.printf("[WOR] Warm Boot active. SX1262 IRQ: 0x%04X\n", irqStatus);
    sxClearIrq();

    // Send BME280 readings to Receiver Node
    readAndSendBME280();
    delay(50);

    // Send the Gatekeeper (alertFlag) to Receiver Node
    sendAlertFlagToReceiver(true);
    delay(50);

    
    
    isFullBoot = false;

    ESP_NOW.end();
    WiFi.mode(WIFI_OFF);
    esp_wifi_stop();

    enterLowPowerWOR();
  }

  // -------------------------------------------------------------
  // 2. SLOW PATH: COLD BOOT / BUTTON / IDE FLASH / RECOVERY
  // -------------------------------------------------------------
  Serial.println("[INIT] Cold Boot active. Full initialization of the SX1262\n");

  if(wakeCause == ESP_SLEEP_WAKEUP_UNDEFINED) {
    //Serial.println("Cold Boot");
    // Hard physical pulse on NRESET line to unlatch radio state machine
    initRadio();

    enterLowPowerWOR();
  }

  sxConfigured = true;  // Radio is operational

  
}

void loop() {
}
