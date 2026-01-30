#include <ArduinoJson.h>

// -------- Pin Definitions --------
#define TRIG_PIN 10
#define ECHO_PIN 9

#define ULTRASONIC_RELAY 12
#define FIRE_RELAY 6
#define SOIL_RELAY 7

#define FIRE_SENSOR_1 3
#define FIRE_SENSOR_2 4
#define SOIL_SENSOR 5

#define IR_SENSOR 2
#define BUZZER 13

// -------- Ultrasonic Settings --------
#define ULTRASONIC_THRESHOLD 10
#define ULTRASONIC_HYSTERESIS 1

// -------- Global States --------
bool irLock = true;
bool motionState = false;

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(ULTRASONIC_RELAY, OUTPUT);
  pinMode(FIRE_RELAY, OUTPUT);
  pinMode(SOIL_RELAY, OUTPUT);

  pinMode(FIRE_SENSOR_1, INPUT);
  pinMode(FIRE_SENSOR_2, INPUT);
  pinMode(SOIL_SENSOR, INPUT);

  pinMode(IR_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(ULTRASONIC_RELAY, HIGH);
  digitalWrite(FIRE_RELAY, HIGH);
  digitalWrite(SOIL_RELAY, HIGH);
  digitalWrite(BUZZER, LOW);
}

void loop() {

  // -------- Read Lock Command --------
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "LOCK_ON") irLock = true;
    if (cmd == "LOCK_OFF") irLock = false;
  }

  // -------- Ultrasonic Distance --------
  long duration;
  float distance;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;

  static bool ultrasonicRelayState = false;

  if (distance > ULTRASONIC_THRESHOLD + ULTRASONIC_HYSTERESIS) {
    ultrasonicRelayState = false;
  } else if (distance > 0 && distance <= ULTRASONIC_THRESHOLD) {
    ultrasonicRelayState = true;
  }

  digitalWrite(ULTRASONIC_RELAY, ultrasonicRelayState ? LOW : HIGH);

  // -------- Fire Sensor --------
  int fire1 = digitalRead(FIRE_SENSOR_1);
  int fire2 = digitalRead(FIRE_SENSOR_2);
  bool fireRelayState = (fire1 == LOW || fire2 == LOW);
  digitalWrite(FIRE_RELAY, fireRelayState ? LOW : HIGH);

  // -------- Soil Sensor --------
  int soil = digitalRead(SOIL_SENSOR);
  bool soilRelayState = (soil == HIGH);
  digitalWrite(SOIL_RELAY, soilRelayState ? LOW : HIGH);

  // -------- IR Motion with Lock --------
  if (irLock) {
    motionState = (digitalRead(IR_SENSOR) == LOW);
    digitalWrite(BUZZER, motionState ? HIGH : LOW);
  } else {
    motionState = false;
    digitalWrite(BUZZER, LOW);
  }

  // -------- Send JSON to NodeMCU --------
  StaticJsonDocument<256> doc;
  doc["distance"] = distance;
  doc["ultrasonicRelay"] = ultrasonicRelayState;
  doc["fireRelay"] = fireRelayState;
  doc["soilRelay"] = soilRelayState;
  doc["motion"] = motionState;
  doc["irLock"] = irLock;

  serializeJson(doc, Serial);
  Serial.println();

  delay(500);
}
