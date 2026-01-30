#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// -------- WiFi Credentials --------
const char* ssid = "HOGWART";
const char* password = "Dracarys!!";

// -------- Web Server --------
ESP8266WebServer server(80);

// -------- States --------
float distance = 0;
bool ultrasonicRelay = false;
bool fireRelay = false;
bool soilRelay = false;
bool motionState = false;
bool irLock = false;   // false = LOCK OFF, true = LOCK ON

// -------- Read JSON from Serial (DO NOT change irLock) --------
void readSerialData() {
  while (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length()) {
      StaticJsonDocument<256> doc;
      if (deserializeJson(doc, line) == DeserializationError::Ok) {
        if (doc.containsKey("distance"))        distance = doc["distance"];
        if (doc.containsKey("ultrasonicRelay")) ultrasonicRelay = doc["ultrasonicRelay"];
        if (doc.containsKey("fireRelay"))       fireRelay = doc["fireRelay"];
        if (doc.containsKey("soilRelay"))       soilRelay = doc["soilRelay"];
        if (doc.containsKey("motion"))          motionState = doc["motion"];
      }
    }
  }
}

// -------- Generate Web Page --------
String generateHTML() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Smart Sensor Dashboard</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#f2f2f2;padding:20px;}";
  html += ".card{background:#fff;padding:20px;margin-bottom:15px;border-radius:10px;box-shadow:0 4px 8px rgba(0,0,0,0.1);}";
  html += ".title{font-size:18px;font-weight:bold;margin-bottom:10px;}";
  html += ".on{color:green;font-weight:bold;} .off{color:red;font-weight:bold;}";
  html += "button{padding:10px 20px;border:none;border-radius:5px;background:#007BFF;color:white;cursor:pointer;}";
  html += "</style></head><body>";

  html += "<div class='card'><div class='title'>Ultrasonic Sensor</div>";
  html += "Distance: " + String(distance) + " cm<br>";
  html += "Relay: <span class='" + String(ultrasonicRelay ? "on" : "off") + "'>";
  html += ultrasonicRelay ? "ON" : "OFF";
  html += "</span></div>";

  html += "<div class='card'><div class='title'>Fire Sensor</div>";
  html += "Relay: <span class='" + String(fireRelay ? "on" : "off") + "'>";
  html += fireRelay ? "ON" : "OFF";
  html += "</span></div>";

  html += "<div class='card'><div class='title'>Soil Sensor</div>";
  html += "Relay: <span class='" + String(soilRelay ? "on" : "off") + "'>";
  html += soilRelay ? "ON (DRY)" : "OFF (WET)";
  html += "</span></div>";

  html += "<div class='card'><div class='title'>IR Motion Sensor</div>";
  html += "Motion: <span class='" + String(motionState ? "on" : "off") + "'>";
  html += motionState ? "DETECTED" : "NONE";
  html += "</span><br>";
  html += "IR Lock: <span class='" + String(irLock ? "on" : "off") + "'>";
  html += irLock ? "ON" : "OFF";
  html += "</span><br><br>";

  // IMPORTANT: POST + redirect-safe button
  html += "<form method='POST' action='/toggle'>";
  html += "<button>";
  html += irLock ? "Deactivate Lock" : "Activate Lock";
  html += "</button></form></div>";

  html += "</body></html>";
  return html;
}

// -------- Toggle IR Lock (NO PAGE RETURN) --------
void handleToggle() {
  irLock = !irLock;

  if (irLock) Serial.println("LOCK_ON");
  else        Serial.println("LOCK_OFF");

  // Redirect to home → refresh will NOT toggle again
  server.sendHeader("Location", "/", true);
  server.send(303);
}

// -------- Setup --------
void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("ESP IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", generateHTML());
  });

  server.on("/toggle", HTTP_POST, handleToggle);

  server.begin();
}

// -------- Loop --------
void loop() {
  server.handleClient();
  readSerialData();
}
