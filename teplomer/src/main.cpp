
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>
#define ONE_WIRE_BUS 4


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

const char* mqtt_server = "147.229.148.105";
int mqtt_port = 11883;
String mqtt_topic = "IoTProject/4/";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

float temperature = 32;
float humidity = 35;

const int reset = 8;

void setup_wifi() {
  bool res;
  WiFiManager wm;
  if(!digitalRead(reset))wm.resetSettings();
  WiFiManagerParameter _mqtt_server("server", "mqtt server", "147.229.148.105", 40);
  wm.addParameter(&_mqtt_server);
  char str[40];
  WiFiManagerParameter _mqtt_port("port", "mqtt port", "11883", 6);
  wm.addParameter(&_mqtt_port);
  WiFiManagerParameter _mqtt_topic("topic", "mqtt topic", "tlacitko", 40);
  wm.addParameter(&_mqtt_topic);
  res = wm.autoConnect("Tlacitko"); 
  strcpy(str, _mqtt_port.getValue());
  mqtt_port = atoi(str);
  strcpy(str,"IoTProject/4/");
  mqtt_server =_mqtt_server.getValue();
  strcat(str , _mqtt_topic.getValue());
  mqtt_topic = str;
  Serial.println(str);
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
      if (client.connect("Teplomer")) {
        Serial.println("connected");
      } else {
        Serial.print("failed");
        delay(5000);
      }
    }
  }

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  sensors.begin();
  pinMode(reset, INPUT_PULLUP);
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
    client.publish(mqtt_topic.c_str(), tempString);

    
  }
}
 