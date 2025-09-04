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
#include <ESP32Servo.h>
#include <Stepper.h>

// ================== Wi-Fi credentials ==================
const char* ssid = "ZzZzZz";
const char* password = "ZzZzZz112233";

// ================== HiveMQ MQTT details ==================
const char* mqtt_server = "c29162f3d8f24ad1ae54157ddb08596c.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "hivemq.webclient.1742322741194";
const char* mqtt_password = "yDw%iF@0Gc7MqSjZ24.,";
const char* mqtt_client_id = "Farm_flow_fr2";

// ================== MQTT Topics ==================
const char* topic_sensors = "sensors/data";
const char* topic_commands = "commands/fr2/fd3";

// ================== Pin Assignments ==================
// Sensors
#define DHT1_PIN 25
#define SOIL1_PIN 33
#define DHT_TYPE DHT11

// Actuators
#define SERVO_PIN    27
#define STEPPER_IN1  14
#define STEPPER_IN2  32
#define STEPPER_IN3  13
#define STEPPER_IN4  12
#define MOTOR_PIN    18

// ================== Stepper Config ==================
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, STEPPER_IN1, STEPPER_IN3, STEPPER_IN2, STEPPER_IN4);

// ================== Sensor Objects ==================
DHT dht1(DHT1_PIN, DHT_TYPE);
BH1750 lightMeter1(0x23);

// ================== Actuator Objects ==================
Servo shadeServo;

// ================== WiFi/MQTT ==================
WiFiClientSecure espClient;
PubSubClient client(espClient);

// ================== Time ==================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// ================== Previous Value Storage ==================
float lastLight1 = -999, lastTemp1 = -999, lastHum1 = -999, lastSoil1 = -999;

// ================== Setup ==================
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  pinMode(DHT1_PIN, INPUT_PULLUP);
  pinMode(MOTOR_PIN, OUTPUT);

  dht1.begin();
  Wire.begin();
  lightMeter1.begin();

  shadeServo.attach(SERVO_PIN);
  myStepper.setSpeed(10);

  setup_wifi();

  timeClient.begin();
  while (!timeClient.update()) {
    timeClient.forceUpdate();
    delay(1000); // Changed to max 1 second delay
  }
  setTime(timeClient.getEpochTime());

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
}

// ================== WiFi ==================
void setup_wifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());
}

// ================== MQTT Reconnect ==================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(topic_commands);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      delay(1000); // Changed to max 1 second delay
    }
  }
}

// ================== MQTT Callback ==================
void mqttCallback(char* topic, byte* message, unsigned int length) {
  Serial.print("MQTT Message arrived on topic: ");
  Serial.println(topic);

  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  Serial.print("Message content: ");
  Serial.println(msg);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msg);
  if (error) {
    Serial.print("JSON deserialization failed: ");
    Serial.println(error.c_str());
    return;
  }

  String actuator = doc["actuator"];
  Serial.print("Actuator requested: ");
  Serial.println(actuator);

  if (actuator == "shade") {
    String status = doc["status"];
    Serial.print("Setting shade servo to status: ");
    Serial.println(status);
    if (status == "on") {
      shadeServo.write(90); // Assuming 90 degrees as "on" position (adjust as needed)
    } else if (status == "off") {
      shadeServo.write(0);  // Assuming 0 degrees as "off" position (adjust as needed)
    }
  } else if (actuator == "stepper") {
    int steps = doc["steps"];
    Serial.print("Moving stepper motor by steps: ");
    Serial.println(steps);
    myStepper.step(steps);
  } else if (actuator == "motor") {
    int speed = doc["speed"];
    Serial.print("Setting motor speed to: ");
    Serial.println(speed);
    if (speed >= 0 && speed <= 255) analogWrite(MOTOR_PIN, speed);
  } else {
    Serial.println("Unknown actuator command received.");
  }
}

// ================== Timestamp ==================
String getTimestamp() {
  char isoTime[30];
  snprintf(isoTime, sizeof(isoTime), "%04d-%02d-%02dT%02d:%02d:%02d.000+00:00",
           year(), month(), day(), hour(), minute(), second());
  return String(isoTime);
}

// ================== Publish Sensor Data ==================
void publishSensorData(const char* farmerId, const char* fieldId,
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
  client.publish(topic_sensors, buffer);
  Serial.printf("Published: %s\n", buffer);
}

// ================== Loop ==================
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float light1 = lightMeter1.readLightLevel();
  float temp1 = dht1.readTemperature();
  float hum1 = dht1.readHumidity();
  int soil1Percent = map(analogRead(SOIL1_PIN), 4095, 0, 0, 100);

  bool shouldPublish1 = (!isnan(light1) && abs(light1 - lastLight1) > 50) ||
                        (!isnan(temp1) && abs(temp1 - lastTemp1) > 0.5) ||
                        (!isnan(hum1) && abs(hum1 - lastHum1) > 1.0) ||
                        (abs(soil1Percent - lastSoil1) > 5);

  if (shouldPublish1 && !isnan(temp1) && !isnan(hum1) && !isnan(light1)) {
    publishSensorData("fr2", "fd3", temp1, hum1, soil1Percent, light1);
    lastLight1 = light1; lastTemp1 = temp1; lastHum1 = hum1; lastSoil1 = soil1Percent;
  }

  static unsigned long lastPublishTime = 0;
  if (millis() - lastPublishTime >= 5000) {
    if (!isnan(temp1) && !isnan(hum1) && !isnan(light1))
      publishSensorData("fr2", "fd3", temp1, hum1, soil1Percent, light1);
    lastPublishTime = millis();
  }
}