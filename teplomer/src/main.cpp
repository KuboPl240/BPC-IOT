
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>
#define ONE_WIRE_BUS 4


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// Replace the next variables with your SSID/Password combination
const char* ssid = "wifi_test";
const char* password = "1203456789";
const char* mqtt_server = "192.168.1.151";
const char* mqtt_port = "1883";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

float temperature = 32;
float humidity = 35;

const int ledPin = 8;

void setup_wifi() {
  bool res;
  WiFiManager wm;
  wm.resetSettings();
  WiFiManagerParameter _mqtt_server("server", "mqtt server", mqtt_server, 40);
  wm.addParameter(&_mqtt_server);
  WiFiManagerParameter _mqtt_port("port", "mqtt port", mqtt_port, 5);
  wm.addParameter(&_mqtt_port);
  mqtt_server = _mqtt_server.getValue();
  //mqtt_port = _mqtt_port.getValue();
  res = wm.autoConnect("Teplomer"); 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  }
  
  void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
  }
  
  void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("Teplomer")) {
        Serial.println("connected");
        // Subscribe
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }

void setup() {
  Serial.begin(115200);
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  //status = bme.begin();  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  sensors.begin();
  pinMode(ledPin, OUTPUT);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    sensors.requestTemperatures(); 
    temperature= sensors.getTempCByIndex(0); 
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    
  }
}
 