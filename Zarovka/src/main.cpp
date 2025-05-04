#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h>


const char* mqtt_server = "147.229.148.105";
int mqtt_port = 11883;
String mqtt_topic = "IoTProject/4/";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];

const int reset= 21;
const int zarovka = 8;
bool zarovkaState = false;


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

    String messageTemp;
    
    for (int i = 0; i < length; i++) {
      messageTemp += (char)message[i];
    }
    Serial.println();
    if (String(topic) == mqtt_topic.c_str()) {
      if(messageTemp == "1"){
        zarovkaState = !zarovkaState;
        digitalWrite(zarovka, zarovkaState);
      }
    }
  }
  
  void reconnect() {
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");
      if (client.connect("Zarovka")) {
        Serial.println("connected");
        client.subscribe(mqtt_topic.c_str());
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
  pinMode(zarovka, OUTPUT);
  pinMode(reset, INPUT_PULLUP);
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}