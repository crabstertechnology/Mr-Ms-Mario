// Predefined Blink Example for ESP32-S3 Mini
#define LED1_PIN 21
#define LED2_PIN 8

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32-S3 Mini Blink Sketch Started!");
}

void loop() {
  // Turn LEDs ON
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, HIGH);
  Serial.println("LEDs ON");
  delay(1000);
  
  // Turn LEDs OFF
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  Serial.println("LEDs OFF");
  delay(1000);
}
