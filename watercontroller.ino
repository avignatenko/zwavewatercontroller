
// note: this is z-uno 5-gen project

#include "ZUNO_DHT.h"
#include "EEPROM.h"

#define CHANNEL_TEMPERATURE 1
#define CHANNEL_HUMIDITY 2
#define CHANNEL_WATER_ALARM 3
#define CHANNEL_WATER_STOP_SWITCH 4

// disable legacy sensor making for water leak sensor
ZUNO_DISABLE(WITH_CC_SENSOR_BINARY);

ZUNO_SETUP_CHANNELS(
  ZUNO_SENSOR_MULTILEVEL_TEMPERATURE(s_temperature),
  ZUNO_SENSOR_MULTILEVEL_HUMIDITY(s_humidity),
  ZUNO_SENSOR_BINARY_WATER(s_water),
  ZUNO_SWITCH_BINARY(getWaterStopSwitch, setWaterStopSwitch));

ZUNO_SETUP_CFGPARAMETER_HANDLER(configParameterChanged);

#define PIN_WATER_ALARM         9
#define PIN_WATER_STOP_SWITCH  10
#define PIN_DHT                11
#define PIN_LED                13

#define EEPROM_ADDR_WATER_STOP_SWITCH 0

// temp & humidity sensor (DHT11)
DHT dht22_sensor(PIN_DHT, DHT11);

byte s_humidity = 0;
byte s_temperature = 0;
byte s_water = 0;
byte s_waterValveBlockSwitch = 0;

byte s_humidityLastReported = 0;
byte s_temperatureLastReported = 0;
unsigned long s_lastReportedTime = 0;


word s_temp_hum_interval = 60 * 20; // 20 mins default, min 30 seconds
word s_temp_threshold = 2;
word s_hum_threshold = 5;


byte getWaterStopSwitch() {
  return s_waterValveBlockSwitch;
}

void setWaterStopSwitch(byte newValue) {
  s_waterValveBlockSwitch = newValue;
  digitalWrite(PIN_WATER_STOP_SWITCH, s_waterValveBlockSwitch > 0 ? HIGH : LOW);
  EEPROM.write(EEPROM_ADDR_WATER_STOP_SWITCH, s_waterValveBlockSwitch);
}

enum {
  CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC = 64,
  CONFIG_TEMPERATURE_THRESHOLD_DEGREES,
  CONFIG_HUMIDITY_THRESHOLD_PERCENT
};

void updateFromCFGParams() {
  s_temp_hum_interval = zunoLoadCFGParam(CONFIG_TEMPERATURE_HUMIDITY_INTERVAL_SEC);
  s_temp_threshold = zunoLoadCFGParam(CONFIG_TEMPERATURE_THRESHOLD_DEGREES);
  s_hum_threshold = zunoLoadCFGParam(CONFIG_HUMIDITY_THRESHOLD_PERCENT);
}

void configParameterChanged(byte param, word value) {
  zunoSaveCFGParam(param, value);
  updateFromCFGParams();
}

void setupLED() {
  pinMode(PIN_LED, OUTPUT);
}

void setupDHT() {
  dht22_sensor.begin();
}

void setupWaterAlarm() {
  pinMode(PIN_WATER_ALARM, INPUT_PULLUP);
}

void setupWaterSwitch() {
  pinMode(PIN_WATER_STOP_SWITCH, OUTPUT);

  byte waterSwitchSaved = EEPROM.read(EEPROM_ADDR_WATER_STOP_SWITCH);
  Serial.print("water switch: ");
  Serial.println(waterSwitchSaved);
  setWaterStopSwitch(waterSwitchSaved);
}


void updateWaterAlarm() {

  byte alarmPin = !digitalRead(PIN_WATER_ALARM);

  if (s_water != alarmPin) {
    s_water = alarmPin;
    zunoSendReport(CHANNEL_WATER_ALARM);

    digitalWrite(PIN_LED, s_water > 0 ? HIGH : LOW);
  }
}

void updateDHT() {
  byte result;
  byte raw_data[5];

  result = dht22_sensor.read(true);

  dht22_sensor.getRawData(raw_data);

  s_humidity = dht22_sensor.readHumidity();
  s_temperature = dht22_sensor.readTemperature();
}

void resetLastReportedData() {
  s_humidityLastReported = s_humidity;
  s_temperatureLastReported = s_temperature;
  s_lastReportedTime = millis();
}

void reportUpdates() {
  bool reportHumidity = (abs(s_humidity - s_humidityLastReported) > s_hum_threshold);
  bool reportTemperature = (abs(s_temperature - s_temperatureLastReported) > s_temp_threshold);

  bool timePassed = (millis() - s_lastReportedTime > (unsigned long) s_temp_hum_interval * 1000);
  
  //Serial.print("time ");
  //Serial.print(s_lastReportedTime);
  //Serial.print(" ");
  //Serial.print(s_temp_hum_interval);
  //Serial.print(" ");
  //Serial.print(millis());
  //Serial.print(" ");
  //Serial.println(timePassed);

  if (reportHumidity || reportTemperature || timePassed) {
    //Serial.print("UPDATE");
    //if (timePassed) Serial.println("TIME");

    zunoSendReport(CHANNEL_TEMPERATURE);
    zunoSendReport(CHANNEL_HUMIDITY);
    resetLastReportedData();
  }
}

void setup() {
  //Serial.begin(115200);

  updateFromCFGParams();

  setupLED();
  setupDHT();
  setupWaterAlarm();
  setupWaterSwitch();

  updateDHT();
  resetLastReportedData();
}



void loop() {

  updateDHT();
  updateWaterAlarm();
  reportUpdates();

  delay(2000);

}
