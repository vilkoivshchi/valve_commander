#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// SSID and pass for your WiFi
const char* ssid = "*SSID*";
const char* password = "*PASSWORD*";

//Define variables
bool valveStatus;
bool valveStatus2;


// Listen port 80

ESP8266WebServer server(80);

//for values at water counters
uint32_t coldWaterCounter;
uint32_t hotWaterCounter;

//state triggers
bool stateCold = false;
bool stateHot = false;

//EEPROM addresses
const uint8_t addrCold = 0;
const uint8_t addrHot = 4;

//pins
const uint8_t coldPin = 14; //D5
const uint8_t hotPin = 12; //D6

//current values at water meters - 1
const uint32_t realCold = 747229;
const uint32_t realHot = 314319;

void setup() {
  Serial.begin(115200);
  pinMode(coldPin, INPUT);
  pinMode(hotPin, INPUT);
  Serial.println("Ready");
  EEPROM.begin(8);
  EEPROM.get(addrCold, coldWaterCounter);
  EEPROM.get(addrHot, hotWaterCounter);

  if (coldWaterCounter < realCold)
  {
    coldWaterCounter = realCold;
    Serial.println("Setting \"real\" cold water counter");
    coldWaterCounter++;
    EEPROM.put(addrCold, coldWaterCounter);
    EEPROM.commit();
  }
  if (hotWaterCounter < realHot)
  {
    hotWaterCounter = realHot;
    Serial.println("Setting \"real\" hot water counter");
    hotWaterCounter++;
    EEPROM.put(addrHot, hotWaterCounter);
    EEPROM.commit();
  }
  if(coldWaterCounter > 99999999)
  {
    coldWaterCounter = 0;
    EEPROM.put(addrCold, coldWaterCounter);
    EEPROM.commit();
    Serial.println("cold water set to 0");

  }
  if(hotWaterCounter > 99999999)
  {
    hotWaterCounter = 0;
    EEPROM.put(addrHot, hotWaterCounter);
    EEPROM.commit();
    Serial.println("hot water set to 0");
  }
  Serial.print("Cold: ");
  Serial.print(coldWaterCounter);
  Serial.print("\n");
  Serial.print("Hot: ");
  Serial.print(hotWaterCounter);
  Serial.print("\n");

  // preparing GPIO, channel 1
  pinMode(5, OUTPUT);
  digitalWrite(5, 1);
  pinMode(4, OUTPUT);
  digitalWrite(4, 1);
  // preparing GPIO, channel 1
  pinMode(0, OUTPUT);
  digitalWrite(0, 1);
  pinMode(2, OUTPUT);
  digitalWrite(2, 1);
  // preparing GPIO, for RESET
  pinMode(16, OUTPUT);
  digitalWrite(16, 1);
   // Binding IP address. 1. device ip addr, 2. gateway, 3 subnet mask
  WiFi.mode(WIFI_STA); // client mode
  WiFi.config(IPAddress(192, 168, 1, 250), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));


  WiFi.begin(ssid, password);
  // awaiting fo connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Show access point and signal strength
  Serial.println(WiFi.BSSIDstr());
  Serial.println(WiFi.RSSI());
  // show ip in console
  Serial.println(WiFi.localIP());
  // setting transmitter power (if nessesary)
  WiFi.setOutputPower(20.5);


  //Bellow defining actions
  server.on("/", []() {

    server.send(200, "text/html","<html><head><title>Valve commander</title></head><body>Available commands:<br>\
    <a href=http://" + WiFi.localIP().toString() + "/open>/open</a><br>\
    <a href=http://" + WiFi.localIP().toString() + "/open2>/open2</a><br>\
    <a href=http://" + WiFi.localIP().toString() + "/close>/close</a><br>\
    <a href=http://" + WiFi.localIP().toString() + "/close2>/close2</a><br>\
    <a href=http://" + WiFi.localIP().toString() + "/closeall>/closeall</a><br>\
    <a href=http://" + WiFi.localIP().toString() + "/rssi>/rssi</a><br>\
    <a href=http://" + WiFi.localIP().toString() + "/bssid>/bssid</a></br>\
    <a href=http://" + WiFi.localIP().toString() + "/coldwater>/coldwater</a></br>\
    <a href=http://" + WiFi.localIP().toString() + "/hotwater>/hotwater</a></body></html>");
  });
  server.on("/open", []() {
    digitalWrite(5, 0);
    digitalWrite(4, 1);
    yield();
    delay(10000);
    digitalWrite(5, 1);
    digitalWrite(4, 1);
    valveStatus = false;
    server.send(200, "text/html", "ON");

  });
  server.on("/close", []() {
    digitalWrite(5, 1);
    digitalWrite(4, 0);
    yield();
    delay(10000);
    digitalWrite(5, 1);
    digitalWrite(4, 1);
    valveStatus = true;
    server.send(200, "text/html", "OFF");

  });
  server.on("/open2", []() {
    digitalWrite(0, 0);
    digitalWrite(2, 1);
    yield();
    delay(10000);
    digitalWrite(0, 1);
    digitalWrite(2, 1);
    valveStatus2 = false;
    server.send(200, "text/html", "ON");

  });
  server.on("/close2", []() {
    digitalWrite(0, 1);
    digitalWrite(2, 0);
    yield();
    delay(10000);
    digitalWrite(0, 1);
    digitalWrite(2, 1);
    valveStatus2 = true;
    server.send(200, "text/html", "OFF");

  });
  server.on("/closeall", []() {
    digitalWrite(0, 1);
    digitalWrite(2, 0);
    digitalWrite(5, 1);
    digitalWrite(4, 0);
    delay(10000);
    yield();
    digitalWrite(0, 1);
    digitalWrite(2, 1);
    digitalWrite(5, 1);
    digitalWrite(4, 1);
    valveStatus = true;
    valveStatus2 = true;
    server.send(200, "text/html", "OFF");

  });
  server.on("/rssi", []() {
    String rssi;
    rssi = WiFi.RSSI();
    server.send(200, "text/html", rssi);
  });

  server.on("/bssid", []() {
    String rssi;
    String bssid;
    bssid = WiFi.BSSIDstr();

    rssi = WiFi.RSSI();

    server.send(200, "text/html", "BSSD: " + bssid + "\n" + "RSSI:" + rssi + " dBm<meta http-equiv=\"refresh\" content=\"10\">");
  });

  server.on("/status", []() {
    if (valveStatus == false) {
      server.send(200, "text/html", "ON");
    }
    else
    {
      server.send(200, "text/html", "OFF");
    }
  });
  server.on("/status2", []() {
    if (valveStatus2 == false) {
      server.send(200, "text/html", "ON");
    }
    else
    {
      server.send(200, "text/html", "OFF");
    }
  }
  );
    server.on("/coldwater", []() {
      char coldWaterToWrite[16];
      sprintf(coldWaterToWrite,"%lu", coldWaterCounter);
    server.send(200, "text/html", coldWaterToWrite);
  });
    server.on("/hotwater", []() {
      char hotWaterToWrite[16];
      sprintf(hotWaterToWrite,"%lu", hotWaterCounter);
    server.send(200, "text/html", hotWaterToWrite);
  });
  // Starting server
  server.begin();
  Serial.println("HTTP server started");
}


void loop() {
  bool triggerCold = digitalRead(coldPin);
  bool triggerHot = digitalRead(hotPin);

  if (triggerCold == HIGH && stateCold == false)
  {
    delay(50);
    //Serial.println(coldWaterCounter);
    coldWaterCounter++;
    EEPROM.put(addrCold, coldWaterCounter);
    EEPROM.commit();
    EEPROM.get(addrCold, coldWaterCounter);
    Serial.print("Cold: ");
    Serial.print(coldWaterCounter);
    Serial.print("\n");
    stateCold = true;
  }
  //else if (trigger == LOW && state == true)
  else if (triggerCold == LOW)
  {
    delay(50);
    stateCold = false;
  }

  if (triggerHot == HIGH && stateHot == false)
  {
    delay(50);
    //Serial.println(coldWaterCounter);
    hotWaterCounter++;
    EEPROM.put(addrHot, hotWaterCounter);
    EEPROM.commit();
    EEPROM.get(addrHot, hotWaterCounter);
    Serial.print("Hot: ");
    Serial.print(hotWaterCounter);
    Serial.print("\n");
    stateHot = true;
  } else if (triggerHot == LOW)
  {
    delay(50);
    stateHot = false;
  }

  //checking wifi status, reset if connection lost.  (pin D0 and RST must be connected)
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection lost. Resetting...");
    ESP.restart();
  }

  server.handleClient();
}
