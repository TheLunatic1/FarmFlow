#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// Wi-Fi credentials
const char* ssid = "ZzZzZz";
const char* password = "ZzZzZz112233";

// HiveMQ Cloud broker details
const char* mqtt_server = "c29162f3d8f24ad1ae54157ddb08596c.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "hivemq.webclient.1742322741194";
const char* mqtt_password = "yDw%iF@0Gc7MqSjZ24.,";
const char* mqtt_client_id = "Farm_flow";

// MQTT topic (single topic for all data)
const char* topic_sensors = "sensors/data";

// Sensor pins
#define DHT1_PIN 25  // Farmer 1, Field 1
#define DHT2_PIN 26  // Farmer 2, Field 1
#define SOIL1_PIN 33 // Farmer 1, Field 1 (ADC1_CH5)
#define SOIL2_PIN 36 // Farmer 2, Field 1 (ADC1_CH0)
#define DHT_TYPE DHT11

// DHT sensors
DHT dht1(DHT1_PIN, DHT_TYPE);
DHT dht2(DHT2_PIN, DHT_TYPE);

// BH1750 sensors
BH1750 lightMeter1(0x23); // Farmer 1, Field 1 (ADDR pin to GND)
BH1750 lightMeter2(0x5C); // Farmer 2, Field 1 (ADDR pin to VCC)

// WiFi and MQTT clients
WiFiClientSecure espClient;
PubSubClient client(espClient);

// NTP Client for time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // Update every 60s

// Variables to track previous values
float lastLight1 = -999, lastTemp1 = -999, lastHum1 = -999, lastSoil1 = -999;
float lastLight2 = -999, lastTemp2 = -999, lastHum2 = -999, lastSoil2 = -999;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  
  // Enable internal pull-up resistors for DHT11 pins
  pinMode(DHT1_PIN, INPUT_PULLUP);
  pinMode(DHT2_PIN, INPUT_PULLUP);
  
  dht1.begin();
  dht2.begin();
  Wire.begin(); // I2C for BH1750
  lightMeter1.begin();
  lightMeter2.begin();
  
  setup_wifi();
  timeClient.begin();
  // Wait for NTP sync
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    delay(500);
  }
  setTime(timeClient.getEpochTime()); // Set system time
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

String getTimestamp() {
  // Get current time components
  char isoTime[30];
  snprintf(isoTime, sizeof(isoTime), "%04d-%02d-%02dT%02d:%02d:%02d.000+00:00",
           year(), month(), day(),
           hour(), minute(), second());
  return String(isoTime);
}

void publishSensorData(const char* topic, const char* farmerId, const char* fieldId,
                      float temperature, float humidity, float soilMoisture, float lightIntensity) {
  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["soil_moisture"] = soilMoisture;
  doc["light_intensity"] = lightIntensity;
  doc["farmerId"] = farmerId;
  doc["fieldId"] = fieldId;
  doc["timeStamp"] = getTimestamp();
  
  char buffer[256];
  serializeJson(doc, buffer);
  client.publish(topic, buffer);
  Serial.print("Published to ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(buffer);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Farmer 1, Field 1
  float light1 = lightMeter1.readLightLevel();
  float temp1 = dht1.readTemperature();
  float hum1 = dht1.readHumidity();
  int soil1Value = analogRead(SOIL1_PIN);
  int soil1Percent = map(soil1Value, 4095, 0, 0, 100);
  
  // Check for significant changes and publish immediately if detected
  bool shouldPublish1 = (!isnan(light1) && abs(light1 - lastLight1) > 50) ||
                       (!isnan(temp1) && abs(temp1 - lastTemp1) > 0.5) ||
                       (!isnan(hum1) && abs(hum1 - lastHum1) > 1.0) ||
                       (abs(soil1Percent - lastSoil1) > 5);
  
  if (shouldPublish1 && !isnan(temp1) && !isnan(hum1) && !isnan(light1)) {
    publishSensorData(topic_sensors, "fr1", "fd1", temp1, hum1, soil1Percent, light1);
    lastLight1 = light1;
    lastTemp1 = temp1;
    lastHum1 = hum1;
    lastSoil1 = soil1Percent;
  }

  // Farmer 2, Field 1
  float light2 = lightMeter2.readLightLevel();
  float temp2 = dht2.readTemperature();
  float hum2 = dht2.readHumidity();
  int soil2Value = analogRead(SOIL2_PIN);
  int soil2Percent = map(soil2Value, 4095, 0, 0, 100);
  
  bool shouldPublish2 = (!isnan(light2) && abs(light2 - lastLight2) > 50) ||
                       (!isnan(temp2) && abs(temp2 - lastTemp2) > 0.5) ||
                       (!isnan(hum2) && abs(hum2 - lastHum2) > 1.0) ||
                       (abs(soil2Percent - lastSoil2) > 5);
  
  if (shouldPublish2 && !isnan(temp2) && !isnan(hum2) && !isnan(light2)) {
    publishSensorData(topic_sensors, "fr2", "fd1", temp2, hum2, soil2Percent, light2);
    lastLight2 = light2;
    lastTemp2 = temp2;
    lastHum2 = hum2;
    lastSoil2 = soil2Percent;
  }

  // Constant publish every 5 seconds
  static unsigned long lastPublishTime = 0;
  if (millis() - lastPublishTime >= 5000) {
    if (!isnan(temp1) && !isnan(hum1) && !isnan(light1)) {
      publishSensorData(topic_sensors, "fr1", "fd1", temp1, hum1, soil1Percent, light1);
    }
    if (!isnan(temp2) && !isnan(hum2) && !isnan(light2)) {
      publishSensorData(topic_sensors, "fr2", "fd1", temp2, hum2, soil2Percent, light2);
    }
    lastPublishTime = millis();
  }
}
