

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#define outputA 10
#define outputB 20
int aState;
int aLastState;
bool setting = true; 
bool heating = false; 

#define OLED_RESET     21 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

const char* mqtt_server = "147.229.148.105";
int mqtt_port = 11883;
String mqtt_topic = "IoTProject/4/";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
float temperature = 0;
float target_temp = 20;

const int reset = 2;

void configMode(WiFiManager *myWiFiManager){
  display.setCursor(0,0);
  display.setTextColor(WHITE, BLACK);
  display.println("Prosim nastavte zariadenie");
  display.println(WiFi.softAPIP());
  display.println(myWiFiManager->getConfigPortalSSID());
  display.display();
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
    res = wm.autoConnect("Termostat"); 
    strcpy(str, _mqtt_port.getValue());
    mqtt_port = atoi(str);
    strcpy(str,"IoTProject/4/");
    mqtt_server =_mqtt_server.getValue();
    strcat(str , _mqtt_topic.getValue());
    mqtt_topic = str;
    delay(10);
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextColor(WHITE, BLACK);
    display.println("WiFi Pripojene");
    display.println("IP addresa: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
  }
  
  void callback(char* topic, byte* message, unsigned int length) {
    String messageTemp;
    
    for (int i = 0; i < length; i++) {
      messageTemp += (char)message[i];
    }
    Serial.println();
    if (String(topic) == mqtt_topic.c_str()) {
      temperature = messageTemp.toFloat();
    }
  }
  
   void mqttconnect(){
    while (!client.connected()) {
      if (client.connect("Termostat")) {
        display.println("MQTT server pripojeny");
        display.display();
        client.subscribe(mqtt_topic.c_str());
        delay(2000);
      }else{
        display.println("Nepodarilo sa pripojit ku MQQT serveru");
        display.display();
        delay(2000);
        display.clearDisplay();
        display.setCursor(0,0);
      }
      delay(1);
    }
   }
  void setup() {
    Serial.begin(115200);
    pinMode (outputA,INPUT);
    pinMode (outputB,INPUT); 
    pinMode (reset,INPUT_PULLUP);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }
    aLastState = digitalRead(outputA); 
    display.display();
    delay(2000); 
    display.clearDisplay();
    display.display();
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
  }
  
  
  void loop() {
    if(temperature<(target_temp-1))heating=1;
    else if (temperature>(target_temp+1))heating=0;
    long now = millis();
    mqttconnect();
    aState = digitalRead(outputA); 
    if (aState != aLastState){     
      setting = true;
      if (digitalRead(outputB) != aState) { 
        target_temp+=0.1;
      } else if(target_temp>0) {
        target_temp-=0.1;
      }
      long now = millis();
      display.clearDisplay();
      display.setCursor(20,0);
      display.setTextColor(WHITE, BLACK);
      display.setTextSize(2);
      display.print("Nastav: ");
      display.setCursor(35,20);
      display.printf("%.1f ",target_temp);
      display.display();
      lastMsg = now;
    } else {
      if(setting){
        setting = false;
        display.clearDisplay();
      }
    }

    aLastState = aState; 
  
  client.loop();
  if ((now - lastMsg > 1000) && (setting==false)) {
    lastMsg = now;
    display.setCursor(20,0);
    display.setTextColor(WHITE, BLACK);
    display.setTextSize(2);
    display.print("Teplota: ");
    display.setCursor(30,20);
    display.printf("%.1f",temperature);
    display.write(0xF8); 
    display.print("C");
    display.drawLine(0,40,128, 40, WHITE);
    display.setTextSize(1);
    display.setCursor(0,43);
    display.printf("Nastavena %.1f C",target_temp);
    if(heating){
      display.fillRect(0, 50, display.width(), 63, WHITE);
      display.setTextColor(BLACK, WHITE);
      display.setTextSize(1);
      display.setCursor(0,52);
      display.print("Kurenie ON ");
    }
    else{
      display.fillRect(0, 50, display.width(), 63, BLACK);
      display.setTextColor(WHITE, BLACK);
      display.setTextSize(1);
      display.setCursor(0,52);
      display.print("Kurenie OFF ");
    }
    display.display();
  }
  }