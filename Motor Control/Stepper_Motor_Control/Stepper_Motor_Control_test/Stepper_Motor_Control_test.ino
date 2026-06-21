// Define the specific pins for the A-Axis on CNC Shield V3
const int stepPinA = 2; 
const int dirPinA = 5;  
const int enPin = 8;     // Shared enable pin for all drivers

const int stepsPerRevolution = 1200; // Change to 2048 if using the 28BYJ-48

void setup() {
  pinMode(stepPinA, OUTPUT);
  pinMode(dirPinA, OUTPUT);
  pinMode(enPin, OUTPUT);
  
  // Enable the drivers (Active LOW)
  digitalWrite(enPin, LOW); 
  
  Serial.begin(9600);
  Serial.println("CNC Shield A-Axis Independent Test...");
}

void loop() {
  int speedDelay = 1000; // Microseconds

  // 1. Spin Clockwise
  Serial.println("A-Axis Moving Clockwise...");
  digitalWrite(dirPinA, HIGH); 
  
  for(int x = 0; x < stepsPerRevolution; x++) {
    digitalWrite(stepPinA, HIGH);
    delayMicroseconds(speedDelay);    
    digitalWrite(stepPinA, LOW);
    delayMicroseconds(speedDelay);    
  }
  
  delay(1000); 

  // 2. Spin Counter-Clockwise
  Serial.println("A-Axis Moving Counter-Clockwise...");
  digitalWrite(dirPinA, LOW); 
  
  for(int x = 0; x < stepsPerRevolution; x++) {
    digitalWrite(stepPinA, HIGH);
    delayMicroseconds(speedDelay); 
    digitalWrite(stepPinA, LOW);
    delayMicroseconds(speedDelay); 
  }
  
  delay(2000); 
}