#define ITR_PIN A1


void setup() {
  pinMode(ITR_PIN,INPUT);
  Serial.begin(9600);
}

void loop() {
  int sensorValue = digitalRead(ITR_PIN);

  if (sensorValue == LOW) {
    Serial.println(1);   // Object detected
  }
  else {
    Serial.println(0);   // No object detected
  }

  delay(100);
}