
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>


unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

const char* mqtt_server = "147.229.148.105";
int mqtt_port = 11883;
String mqtt_topic = "IoTProject/4/";

struct Button {
  const uint8_t PIN;
  bool pressed;
};

Button button1 = {21, false};

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

const int reset= 8;

void IRAM_ATTR isr() {
  button_time = millis();
if (button_time - last_button_time > 250)
{
      button1.pressed = true;
     last_button_time = button_time;
}
}

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
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");
      Serial.println(mqtt_port);
      Serial.println(mqtt_topic);
      if (client.connect("Tlacitko")) {
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
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  pinMode(reset, INPUT_PULLUP);
  pinMode(button1.PIN, INPUT_PULLUP);
  attachInterrupt(button1.PIN, isr, FALLING);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (button1.pressed) {
    button1.pressed = false;
    client.publish(mqtt_topic.c_str(), "1");
  }
}
