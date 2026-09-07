#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(D6, D7);

// --- Flood detection pins
int FloatSensor = D1;
const int trigPin1 = D4;
const int echoPin1 = D3;

const String alertNumber = "+8801XXXXXXXXX";  // Replace with your alert number
const String rescueNumber = "+8801XXXXXXXXX"; // Replace with your rescue number

unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 10000;
long duration1;
int distance1;
int buttonState = 1;
bool alertSent_FloatSensor = false;
bool alertSent_Distance = false;

// --- MPU6050 Earthquake/Shake Detection
Adafruit_MPU6050 mpu;

int samples = 9;
#define maxVal1 3   // earthquake/vibration positive threshold
#define minVal1 -3  // earthquake/vibration negative threshold

float xcal = 0, ycal = 0, zcal = 0;
bool alertSent_Quake = false;

void setup() {
  Serial.begin(9600);
  gsmSerial.begin(9600);
  Wire.begin(D2, D1);
  pinMode(FloatSensor, INPUT_PULLUP);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  Serial.println("SMART DAM + EARTHQUAKE SYSTEM INITIALIZING...");
  delay(1000);
  startMillis = millis();

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) delay(10);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Calibration (average first 9 samples)
  Serial.println("Calibrating MPU6050...");
  for (int i = 0; i < samples; i++) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    xcal += a.acceleration.x;
    ycal += a.acceleration.y;
    zcal += a.acceleration.z;
    delay(100);
  }
  xcal /= samples;
  ycal /= samples;
  zcal /= samples;
  Serial.print("Calibration done. x0="); Serial.print(xcal,2); Serial.print(" y0="); Serial.print(ycal,2); Serial.print(" z0="); Serial.println(zcal,2);
}

void loop() {
  // ===========================
  // === Flood Detection ===
  // ===========================
  // Ultrasonic distance measurement
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = duration1 * 0.034 / 2;
  Serial.print("Distance: ");
  Serial.println(distance1);

  // Float sensor reading (HIGH = water present)
  buttonState = digitalRead(FloatSensor);

  // High water level alert (float)
  if (buttonState == HIGH) {
    if (!alertSent_FloatSensor) {
      Serial.println("WATER LEVEL - HIGH");
      sendMessage(alertNumber, "Flood Alert! Water level has crossed the danger limit.");
      sendMessage(rescueNumber, "URGENT RESCUE ALERT: Water level at the dam is critical. Immediate assistance required.");
      alertSent_FloatSensor = true;
    }
  } else {
    alertSent_FloatSensor = false;
    Serial.println("WATER LEVEL - LOW");
    delay(300);
  }

  // Distance-based alert
  if (distance1 < 10) {
    if (!alertSent_Distance) {
      sendMessage(alertNumber, "Flood Alert! Water level has crossed the 10% danger limit.");
      sendMessage(rescueNumber, "RESCUE TEAM: Water level is critically high (10% over threshold). Prepare for deployment.");
      alertSent_Distance = true;
    }
  } else {
    alertSent_Distance = false;
  }

  // ===========================
  // === Earthquake Detection ===
  // ===========================
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float xValue = xcal - a.acceleration.x;
  float yValue = ycal - a.acceleration.y;
  float zValue = zcal - a.acceleration.z;

  Serial.print("xDiff="); Serial.print(xValue,2);
  Serial.print(" yDiff="); Serial.print(yValue,2);
  Serial.print(" zDiff="); Serial.print(zValue,2);
  Serial.println();

  // If any axis difference is outside threshold, SIGNAL EARTHQUAKE
  if ((xValue < minVal1 || xValue > maxVal1) ||
      (yValue < minVal1 || yValue > maxVal1) ||
      (zValue < minVal1 || zValue > maxVal1)) {

    if (!alertSent_Quake) {
      Serial.println("EARTHQUAKE DETECTED!");
      sendMessage(alertNumber, "Earthquake ALERT! Strong vibration detected at the dam site.");
      sendMessage(rescueNumber, "URGENT: Earthquake detected at the dam. Mobilize rescue response.");
      alertSent_Quake = true;
    }
  } else {
    alertSent_Quake = false;
  }

  // Loop timing
  currentMillis = millis();
  if (currentMillis - startMillis >= period) {
    startMillis = currentMillis;
  }

  delay(200);
}

void sendMessage(String phoneNumber, String message) {
  Serial.print("Attempting to send SMS to ");
  Serial.println(phoneNumber);

  gsmSerial.println("AT+CMGF=1");   
  delay(500);

  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(500);

  if (gsmSerial.find(">")) {
    Serial.println("Module ready to send message...");
    gsmSerial.print(message);
    delay(200);
    gsmSerial.write(26);  // Ctrl+Z
    Serial.println("Sending...");

    unsigned long start = millis();
    bool okFound = false;

    while (millis() - start < 7000) {
      if (gsmSerial.available()) {
        String resp = gsmSerial.readString();
        Serial.print("GSM Response: ");
        Serial.println(resp);

        if (resp.indexOf("OK") != -1) {
          okFound = true;
          break;
        }
      }
    }

    if (okFound) {
      Serial.println(" SMS sent successfully.");
    } else {
      Serial.println(" SMS failed or timeout. (But message may still send)");
    }

  } else {
    Serial.println(" GSM module not ready (no > prompt).");
  }
}
