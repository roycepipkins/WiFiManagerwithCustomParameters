#include "SPIFFS.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         
#include <ArduinoJson.h>          
#include <M5Stack.h>
#include <vector>

using namespace std;

#define TFT_GREY 0x5AEB

WiFiManagerParameter Voltage("Voltage", "Voltage", "Voltage", 8);
WiFiManagerParameter Frequency("Frequency", "Frequency", "Frequency", 10);
WiFiManagerParameter Duty_Cycle("Duty Cycle", "Duty Cycle", "Duty Cycle", 11);


template <typename ArgType> void MirrorPrint(ArgType arg)
{
  Serial.print(arg);
  M5.Lcd.print(arg);
}


template <typename ArgType> void MirrorPrintln(ArgType arg)
{
  Serial.println(arg);
  M5.Lcd.println(arg);
}



//flag for saving data
bool shouldSaveConfig = true;

//callback notifying us of the need to save config
void saveConfigCallback () {
  MirrorPrintln("Should save config");
  shouldSaveConfig = true;
}

void eraseConfig()
{
  //Note: WiFi and SPIFFS must have already been started
  WiFi.disconnect(true); 
  SPIFFS.begin(true);
  SPIFFS.format();  
  M5.Lcd.fillScreen(TFT_GREY);
  M5.Lcd.setCursor(0, 0, 2);
  MirrorPrintln("Erased Wifi info");
  while(1) delay(1000);
}





void setup() {
  Serial.begin(115200);
  M5.begin();

  

  if (SPIFFS.begin(true)) {
    if (SPIFFS.exists("/config.json")) {
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        size_t size = configFile.size();
        vector<char> buf(size);
        configFile.readBytes(buf.data(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.data());
        if (json.success()) {
          if (json.containsKey("Voltage"))
            Voltage = WiFiManagerParameter("Voltage", "Voltage", json["Voltage"], strlen(json["Voltage"]) + 1);
          if (json.containsKey("Frequency"))
            Frequency = WiFiManagerParameter("Frequency", "Frequency", json["Frequency"], strlen(json["Frequency"]) + 1);
          if (json.containsKey("Duty_Cycle"))
            Duty_Cycle = WiFiManagerParameter("Duty_Cycle", "Duty_Cycle", json["Duty_Cycle"], strlen(json["Duty_Cycle"]) + 1);
        } else {
          MirrorPrintln("failed to load json config");
        }
      }
    }
    else
    {
       MirrorPrintln("No config file");
    }
  } else {
     MirrorPrintln("failed to mount FS");
  }
  //end read
  
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);


  //add all your parameters here
  wifiManager.addParameter(&Voltage);
  wifiManager.addParameter(&Frequency);
  wifiManager.addParameter(&Duty_Cycle);

  M5.Lcd.fillScreen(TFT_GREY);
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextColor(TFT_WHITE,TFT_BLACK);  
  M5.Lcd.setTextSize(1);

  MirrorPrintln("If device is not online in a few seconds");
  MirrorPrintln("then connect to SSID: ");
  MirrorPrintln("ESP" + String(ESP_getChipId()));
 
  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point 
  //named after the chip ID
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    MirrorPrintln("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  MirrorPrintln("connected to WiFi");

  

  //save the custom parameters to FS
  if (shouldSaveConfig) {
    MirrorPrintln("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["Voltage"] = Voltage.getValue();
    json["Frequency"] = Frequency.getValue();
    json["Duty_Cycle"] = Duty_Cycle.getValue();
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      MirrorPrintln("failed to open config file for writing");
    }
    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    Serial.println();
    //end save
  }
}

void loop() {

  M5.Lcd.fillScreen(TFT_GREY);
  M5.Lcd.setCursor(0, 0, 2);
  MirrorPrint("SSID: ");
  MirrorPrintln(WiFi.SSID());
  MirrorPrint("IP: ");
  MirrorPrintln(WiFi.localIP());
  MirrorPrint("Voltage: ");
  MirrorPrintln(Voltage.getValue());
  MirrorPrint("Frequency: ");
  MirrorPrintln(Frequency.getValue());
  MirrorPrint("Duty Cycle: ");
  MirrorPrintln(Duty_Cycle.getValue());

  //eraseConfig();
  
  while(1)
  {
    delay(1000);
  }
///////////////////////////////////////////////////////////////////////


}
