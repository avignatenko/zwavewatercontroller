#include "ZUNO_DHT.h"

ZUNO_SETUP_CHANNELS(
  ZUNO_SENSOR_MULTILEVEL_TEMPERATURE(s_temperature),
  ZUNO_SENSOR_MULTILEVEL_HUMIDITY(s_humidity),
  ZUNO_SENSOR_BINARY_WATER(s_water),
  ZUNO_SWITCH_BINARY(getWaterStopSwitch, setWaterStopSwitch));

#define PIN_WATER_ALARM   9
#define PIN_WATER_SWITCH  10
#define PIN_DHT           11

// temp & humidity sensor (DHT11)
DHT dht22_sensor(PIN_DHT, DHT11);

byte s_humidity = 0;
byte s_temperature = 0;
byte s_water = 0;
byte s_waterSwitch = 0;

byte getWaterStopSwitch() {
    return s_waterSwitch;
}

void setWaterStopSwitch(byte newValue) {
    s_waterSwitch = newValue;
    digitalWrite(PIN_WATER_SWITCH, s_waterSwitch > 0 ? HIGH : LOW);
}

void setupDHT() {
  dht22_sensor.begin();
}

void setupWaterAlarm() {
   pinMode(PIN_WATER_ALARM, INPUT_PULLUP);
}

void setupWaterSwitch() {
  pinMode(PIN_WATER_SWITCH, OUTPUT);
  digitalWrite(PIN_WATER_SWITCH, LOW);
}

void setup() {

  Serial.begin(115200);

  setupDHT();
  setupWaterAlarm();
  setupWaterSwitch();
}


void updateWaterAlarm() {

  byte alarmPin = !digitalRead(PIN_WATER_ALARM);

      Serial.print("water alarm:");
    Serial.println(alarmPin);
    
  if (s_water != alarmPin) {
    s_water = alarmPin;
    zunoSendReport(3);

  }
}

void updateDHT() {
  byte result;
  byte i;
  byte  raw_data[5];

  Serial.print("Millis:");
  Serial.println(millis());


  result = dht22_sensor.read(true);
  Serial.print("DHT read result:");
  Serial.println(result);

  Serial.print("Raw data: { ");
  dht22_sensor.getRawData(raw_data);
  for (i = 0; i < 5; i++)
  {
    Serial.print(raw_data[i], HEX);
    Serial.print(" ");
  }
  Serial.println("} ");

  Serial.print("Temperature:");
  Serial.println(dht22_sensor.readTemperature());
  Serial.print("Humidity:");
  Serial.println(dht22_sensor.readHumidity());

  s_humidity = dht22_sensor.readHumidity();
  s_temperature = dht22_sensor.readTemperature();
}

void loop() {



  updateDHT();
  updateWaterAlarm();

  delay(3000);



}
