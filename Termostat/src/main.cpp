

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

#define OLED_RESET     21 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

const char* ssid = "wifi_test";
const char* password = "1203456789";

const char* mqtt_server = "192.168.1.151";
const char* mqtt_port = "1883";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
float temperature = 0;
float target_temp = 20;

const int ledPin = 2;

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
    wm.setAPCallback(configMode);
    wm.resetSettings();
    WiFiManagerParameter _mqtt_server("server", "mqtt server", mqtt_server, 40);
    wm.addParameter(&_mqtt_server);
    WiFiManagerParameter _mqtt_port("port", "mqtt port", mqtt_port, 5);
    wm.addParameter(&_mqtt_port);
    mqtt_server = _mqtt_server.getValue();
    mqtt_port = _mqtt_port.getValue();
    res = wm.autoConnect("Termostat"); 
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
    if (String(topic) == "esp32/temperature") {
      temperature = messageTemp.toFloat();
    }
  }
  
   void mqttconnect(){
    while (!client.connected()) {
      if (client.connect("Termostat")) {
        display.println("MQTT server pripojeny");
        display.display();
        client.subscribe("esp32/temperature");
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
    aState = digitalRead(outputA); // Reads the "current" state of the outputA
    // If the previous and the current state of the outputA are different, that means a Pulse has occured
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

    aLastState = aState; // Updates the previous state of the outputA with the current state
  
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