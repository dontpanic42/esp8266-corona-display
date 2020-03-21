#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>        // Include the Wi-Fi library
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
StaticJsonDocument<256> jsonDoc;

const char* ssid = "YOUR_SSID" // Set to your WiFi's SSID;
const char* password = "YOUR_WIFI_PASSWORD" // Set to your WiFi's Password;

const char* apiHost = "corona.lmao.ninja";
const char* apiCountry = "germany";
const int apiPort = 443;

void simplePrint(const char * msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 16);
  display.println(msg);
  display.display(); 
}

void initDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
}

void initWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  simplePrint("Connecting...");
  
  int i = 0;
  while(WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      simplePrint("E: SSID not found");
      for(;;){}
    }

    if (WiFi.status() == WL_CONNECT_FAILED) {
      simplePrint("E: Wrong password");
      for(;;){}
    }
    
    Serial.print(++i); Serial.print(' ');
    delay(1000);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.localIP());

  simplePrint("Connected!");
  delay(2000);
}

void fetchStats() {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  if(https.begin(client, apiHost, apiPort, String("/countries/") + apiCountry)) {
    Serial.println(String("Connected to ") + apiHost); 

    int returnCode = https.GET();
    if(returnCode > 0) {
      String payload = https.getString();
      Serial.println(String("HTTP Code: ") + returnCode);
      Serial.println(payload);
      if (returnCode != 200) {
        Serial.println("Error: Non 200 return code");
        simplePrint("E: Return Code");
        return;
      } 

      displayResponse(payload);
    }
  } else {
    Serial.println(String("Failed to connect to ") + apiHost);
    simplePrint("E: Server Connection");
  }
}

void displayResponse(String & payload) {
  deserializeJson(jsonDoc, payload);

  Serial.println((int) jsonDoc["cases"]);
  Serial.println((int) jsonDoc["active"]);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(String("Total: ") + (int) jsonDoc["cases"]);

  display.setCursor(0, 16);
  display.println(String("Active: "));
  
  display.setCursor(0, 30);
  display.setTextSize(3);
  display.println((int) jsonDoc["active"]);
  
  display.setCursor(0, 56);
  display.setTextSize(1);
  display.println(String("New Today:") + (int) jsonDoc["todayCases"]);
  
  display.display(); 

}

void setup() {
  Serial.begin(9600);

  initDisplay();

  initWifi();
}

void loop() {
  // Check for connection issues, reconnect if necessary
  if(WiFi.status() != WL_CONNECTED) {
    simplePrint("Connection lost");
    delay(5000);
    initWifi();
  }

  fetchStats();

  delay(60000);
}
