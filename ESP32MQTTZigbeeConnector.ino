#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>

const int sensorPin = 3;
HardwareSerial Zigbee(2);

const char* ssid = "ACL-WLAN";
const char* password = "welcome2ACL";

const char* mqtt_server = "10.4.58.59";
const int mqtt_port = 1883;
const char* mqtt_user = "mqtt-user";
const char* mqtt_password = "user";
const char* mqtt_topic = "zigbee2mqtt";
const char* mqtt_topicTwo = "originalValue";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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

void mqtt_callback(char* topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  Serial.println();
}

void mqtt_reconnect() {

  while (!client.connected()) {

    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {

      Serial.println("connected");
      client.subscribe("zigbee/commands");
    }
    else {

      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  Zigbee.begin(9600, SERIAL_8N1, 16, 17);

  Serial.println("Setup abgeschlossen");

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);
}

void loop() {

  int sensorValue = analogRead(sensorPin);

  float finalValue = sensorValue;
  finalValue /= 4095;
  finalValue = 50;
  finalValue= 240;
  finalValue /= 1000;
  float energy = finalValue;

  Serial.print("Sensorwert: ");
  Serial.print(energy);
  Serial.println(" kWh");

  char payload[50];
  sprintf(payload, "Sensorwert: %.2f kWh", energy);
  Zigbee.println(payload);

  if (!client.connected()) {

    mqtt_reconnect();
  }

  client.loop();
  client.publish(mqtt_topic, payload);

  Serial.print("Sensorwert: ");
  Serial.println(sensorValue);

  payload[50];
  sprintf(payload, "Sensorwert: %f", sensorValue);
  Zigbee.println(payload);

  if (!client.connected()) {

    mqtt_reconnect();
  }

  client.loop();
  client.publish(mqtt_topicTwo, payload);

  delay(500);
}