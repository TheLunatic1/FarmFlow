// Upload fake data to HiveMQ Cloud for testing purposes, 
// simulating the behavior of the light intensity (LDR), DHT11 (temperature and humidity), and soil moisture sensors
// Across the three fields (Farmer 1: Field 1 & Field 2, Farmer 2: Field 1). 
// The fake data will be generated programmatically.
// And commented out the actual sensor reading code while keeping it in place for reference.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// #include <DHT.h>


const char* ssid = "ZzZzZz";
const char* password = "ZzZzZz112233";


const char* mqtt_server = "e5e391a1421d4659877b9a52166e049e.s1.eu.hivemq.cloud"; // HiveMQ Cloud hostname
const int mqtt_port = 8883;                             // TLS port for HiveMQ Cloud
const char* mqtt_user = "Farm_flow"; // MQTT username
const char* mqtt_password = "Farm_flow11223";     // MQTT password
const char* mqtt_client_id = "Farm_flow";                // unique client ID


const char* topic_f1_field1 = "farmer1_field1/data";
const char* topic_f1_field2 = "farmer1_field2/data";
const char* topic_f2_field1 = "farmer2_field1/data";

// Sensor pins
// #define LDR1_PIN 34  // Farmer 1, Field 1 (ADC1_CH6)
// #define LDR2_PIN 35  // Farmer 1, Field 2 (ADC1_CH7)
// #define LDR3_PIN 32  // Farmer 2, Field 1 (ADC1_CH4)
// #define DHT1_PIN 25  // Farmer 1, Field 1
// #define DHT2_PIN 26  // Farmer 1, Field 2
// #define DHT3_PIN 27  // Farmer 2, Field 1
// #define SOIL1_PIN 33 // Farmer 1, Field 1 (ADC1_CH5)
// #define SOIL2_PIN 36 // Farmer 1, Field 2 (ADC1_CH0)
// #define SOIL3_PIN 39 // Farmer 2, Field 1 (ADC1_CH3)
// #define DHT_TYPE DHT11
// #define THRESHOLD 2048

// DHT sensors
// DHT dht1(DHT1_PIN, DHT_TYPE);
// DHT dht2(DHT2_PIN, DHT_TYPE);
// DHT dht3(DHT3_PIN, DHT_TYPE);


WiFiClientSecure espClient;
PubSubClient client(espClient);


int lastLdr1State = -1;
int lastLdr2State = -1;
int lastLdr3State = -1;
float lastTemp1 = -999, lastHum1 = -999, lastSoil1 = -999;
float lastTemp2 = -999, lastHum2 = -999, lastSoil2 = -999;
float lastTemp3 = -999, lastHum3 = -999, lastSoil3 = -999;


unsigned long counter = 0;

void setup() {
  Serial.begin(115200);
  // analogReadResolution(12);
  // dht1.begin();
  // dht2.begin();
  // dht3.begin();
  setup_wifi();
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
  // Placeholder: Use NTP for accurate timestamps
  return "2025-04-27T12:00:00Z";
}

// Function to generate sensor data
void getFakeData(int &ldrState, float &temp, float &hum, int &soilMoisture) {
  // Simulate LDR: Toggle between 0 and 1 every 5 iterations
  ldrState = (counter % 10 < 5) ? 1 : 0;
  
  // Simulate temperature (20–30°C)
  temp = 20 + (counter % 11);
  
  // Simulate humidity (40–80%)
  hum = 40 + (counter % 41);
  
  // Simulate soil moisture (30–70%)
  soilMoisture = 30 + (counter % 41);
}

void publishFieldData(const char* topic, const char* field, int ldrState, float temp, float hum, int soilMoisture) {
  StaticJsonDocument<200> doc;
  doc["field"] = field;
  doc["timeStamp"] = getTimestamp();
  doc["humidity"] = hum;
  doc["temperature"] = temp;
  doc["light"] = ldrState;
  doc["soilMoisture"] = soilMoisture;
  
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

  counter++;

  // Farmer 1, Field 1
  int ldr1State;
  float temp1, hum1;
  int soil1Percent;
  getFakeData(ldr1State, temp1, hum1, soil1Percent);
  // Actual sensor readings
  // int ldr1Value = analogRead(LDR1_PIN);
  // int ldr1State = (ldr1Value < THRESHOLD) ? 1 : 0;
  // float temp1 = dht1.readTemperature();
  // float hum1 = dht1.readHumidity();
  // int soil1Value = analogRead(SOIL1_PIN);
  // int soil1Percent = map(soil1Value, 4095, 0, 0, 100);
  if (ldr1State != lastLdr1State || abs(temp1 - lastTemp1) > 0.5 || abs(hum1 - lastHum1) > 1.0 || abs(soil1Percent - lastSoil1) > 5) {
    // if (!isnan(temp1) && !isnan(hum1)) { // No need to check for nan with fake data
    publishFieldData(topic_f1_field1, "farmer1_field1", ldr1State, temp1, hum1, soil1Percent);
    lastLdr1State = ldr1State;
    lastTemp1 = temp1;
    lastHum1 = hum1;
    lastSoil1 = soil1Percent;
    // }
  }

  // Farmer 1, Field 2
  int ldr2State;
  float temp2, hum2;
  int soil2Percent;
  getFakeData(ldr2State, temp2, hum2, soil2Percent);
  // Actual sensor readings
  // int ldr2Value = analogRead(LDR2_PIN);
  // int ldr2State = (ldr2Value < THRESHOLD) ? 1 : 0;
  // float temp2 = dht2.readTemperature();
  // float hum2 = dht2.readHumidity();
  // int soil2Value = analogRead(SOIL2_PIN);
  // int soil2Percent = map(soil2Value, 4095, 0, 0, 100);
  if (ldr2State != lastLdr2State || abs(temp2 - lastTemp2) > 0.5 || abs(hum2 - lastHum2) > 1.0 || abs(soil2Percent - lastSoil2) > 5) {
    // if (!isnan(temp2) && !isnan(hum2)) {
    publishFieldData(topic_f1_field2, "farmer1_field2", ldr2State, temp2, hum2, soil2Percent);
    lastLdr2State = ldr2State;
    lastTemp2 = temp2;
    lastHum2 = hum2;
    lastSoil2 = soil2Percent;
    // }
  }

  // Farmer 2, Field 1
  int ldr3State;
  float temp3, hum3;
  int soil3Percent;
  getFakeData(ldr3State, temp3, hum3, soil3Percent);
  // Actual sensor readings 
  // int ldr3Value = analogRead(LDR3_PIN);
  // int ldr3State = (ldr3Value < THRESHOLD) ? 1 : 0;
  // float temp3 = dht3.readTemperature();
  // float hum3 = dht3.readHumidity();
  // int soil3Value = analogRead(SOIL3_PIN);
  // int soil3Percent = map(soil3Value, 4095, 0, 0, 100);
  if (ldr3State != lastLdr3State || abs(temp3 - lastTemp3) > 0.5 || abs(hum3 - lastHum3) > 1.0 || abs(soil3Percent - lastSoil3) > 5) {
    // if (!isnan(temp3) && !isnan(hum3)) {
    publishFieldData(topic_f2_field1, "farmer2_field1", ldr3State, temp3, hum3, soil3Percent);
    lastLdr3State = ldr3State;
    lastTemp3 = temp3;
    lastHum3 = hum3;
    lastSoil3 = soil3Percent;
    // }
  }

  delay(5000);
}
