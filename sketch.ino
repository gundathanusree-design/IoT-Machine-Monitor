#include <Wire.h>
#include <DHT.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>

#define DHTPIN 4
#define DHTTYPE DHT22

#define POT_PIN 34
#define BUZZER 25
#define LED 26

DHT dht(DHTPIN, DHTTYPE);
Adafruit_MPU6050 mpu;
LiquidCrystal_I2C lcd(0x27, 16, 2);

float tempLimit = 40.0;
float vibrationLimit = 2.0;
float currentLimit = 1.8;

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  pinMode(LED, OUTPUT);

  Wire.begin(21, 22);

  dht.begin();

  if (!mpu.begin()) {
    Serial.println("MPU6050 not found!");
    while (1) delay(10);
  }

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("IIoT MACHINE");
  lcd.setCursor(0, 1);
  lcd.print("MONITOR SYSTEM");

  delay(2000);
}

void loop() {

  // 1. Read temperature
  float temperature = dht.readTemperature();

  if (isnan(temperature)) {
    Serial.println("DHT sensor error");
    delay(2000);
    return;
  }

  // 2. Read vibration
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  float magnitude = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );

  float vibration = fabs(magnitude - 9.81);

  // 3. Simulate motor current
  int potValue = analogRead(POT_PIN);
  float current = (potValue / 4095.0) * 2.5;

  // 4. Fault detection
  bool tempFault = temperature > tempLimit;
  bool vibrationFault = vibration > vibrationLimit;
  bool currentFault = current > currentLimit;

  int faults = tempFault + vibrationFault + currentFault;

  String status;

  if (faults >= 2) {
    status = "CRITICAL";
  }
  else if (faults == 1) {
    status = "WARNING";
  }
  else {
    status = "NORMAL";
  }

  // 5. Alert system
  digitalWrite(LED, faults > 0 ? HIGH : LOW);

  if (faults > 0) {
    tone(BUZZER, 1000);
  }
  else {
    noTone(BUZZER);
  }

  // 6. LCD output
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print(" V:");
  lcd.print(vibration, 1);

  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(current, 1);
  lcd.print("A ");
  lcd.print(status);

  // 7. Serial monitor output
  Serial.println("==================");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Vibration: ");
  Serial.print(vibration);
  Serial.println(" m/s2");

  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" A");

  Serial.print("Machine Status: ");
  Serial.println(status);

  delay(2000);
}
