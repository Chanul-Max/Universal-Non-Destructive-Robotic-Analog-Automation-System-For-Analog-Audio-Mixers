// Define the Arduino pins connected to the A4988
const int stepPin = 2;
const int dirPin = 3;

// 28BYJ-48 has 2048 steps per full 360-degree revolution (in full-step mode)
const int stepsPerRevolution = 2048; 

void setup() {
  // Configure the driver pins as outputs
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  // Start serial monitor for debugging
  Serial.begin(9600);
  Serial.println("Stepper Motor Test Starting...");
}

void loop() {
  // --------------------------------------------------
  // 1. Spin Clockwise
  // --------------------------------------------------
  Serial.println("Spinning Clockwise...");
  digitalWrite(dirPin, HIGH); // Set direction to Clockwise
  
  // Loop 2048 times to complete one full revolution
  for(int x = 0; x < stepsPerRevolution; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000);    // Wait 2 milliseconds (Speed control)
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000);    // Wait 2 milliseconds
  }
  
  delay(1000); // Wait for 1 second

  // --------------------------------------------------
  // 2. Spin Counter-Clockwise
  // --------------------------------------------------
  Serial.println("Spinning Counter-Clockwise...");
  digitalWrite(dirPin, LOW); // Set direction to Counter-Clockwise
  
  for(int x = 0; x < stepsPerRevolution; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(1000); 
    digitalWrite(stepPin, LOW);
    delayMicroseconds(1000); 
  }
  
  delay(1000); // Wait for 1 second before repeating the loop
}