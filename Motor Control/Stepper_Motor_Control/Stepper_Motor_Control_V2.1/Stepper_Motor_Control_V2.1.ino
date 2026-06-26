// --- CNC Shield V3 Pin Definitions ---
const int enPin = 8;

// X-Axis (fader01)
const int stepX = 2;
const int dirX  = 5;

// Y-Axis (fader02)
const int stepY = 3;
const int dirY  = 6;

// Z-Axis (gain01)
const int stepZ = 4;
const int dirZ  = 7;

// A-Axis (gain02)
const int stepA = 12;
const int dirA  = 13;

// --- ITR Sensor Pins ---
const int itrPin1 = A0; // Sensor for Fader 01
const int itrPin2 = A1; // Sensor for Fader 02

// --- Conversion Constants ---
const float stepsPerMM = 1024.0;
const float stepsPerDegree = 5.6888;

// Motor Speed
const int speedDelay = 600;

// Store CURRENT absolute positions of each motor
float currentPosFader01 = 0;
float currentPosFader02 = 0;
float currentPosGain01  = 0;
float currentPosGain02  = 0;

// --- System State ---
// 0 = Waiting, 1 = Calibration, 2 = Normal Operation
int systemMode = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("System Ready.");
  Serial.println("Send '1' to run Calibration (Distance Measurement).");
  Serial.println("Send '2' to enter Standard Operational Mode.");

  pinMode(enPin, OUTPUT);
  
  pinMode(stepX, OUTPUT); pinMode(dirX, OUTPUT);
  pinMode(stepY, OUTPUT); pinMode(dirY, OUTPUT);
  pinMode(stepZ, OUTPUT); pinMode(dirZ, OUTPUT);
  pinMode(stepA, OUTPUT); pinMode(dirA, OUTPUT);

  // Configure ITR Sensor Pins
  pinMode(itrPin1, INPUT);
  pinMode(itrPin2, INPUT);

  // Enable drivers
  digitalWrite(enPin, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    String commandLine = Serial.readStringUntil('\n');
    commandLine.trim();

    if (commandLine.length() > 0) {
      
      // Mode Switching Logic
      if (commandLine == "1") {
        runCalibration();
        systemMode = 0; 
      } 
      else if (commandLine == "2") {
        systemMode = 2;
        Serial.println("Mode 2 Active. Format: <device>,<absolute_position>");
      } 
      // If we are in Mode 2, parse the command normally
      else if (systemMode == 2) {
        parseAndExecute(commandLine);
      } 
      else {
        Serial.println("Error: Send '1' or '2' first.");
      }
    }
  }
}

// ------------------------------------------------------------------
// PROGRAMME 1: Calibration & Distance Measurement
// ------------------------------------------------------------------
void runCalibration() {
  digitalWrite(dirX, HIGH);
  digitalWrite(dirY, HIGH); 

  long stepsFader1 = 0;
  long stepsFader2 = 0;
  
  bool fader1Done = false;
  bool fader2Done = false;

  while (!fader1Done || !fader2Done) {
    
    if (!fader1Done && digitalRead(itrPin1) == HIGH) fader1Done = true;
    if (!fader2Done && digitalRead(itrPin2) == HIGH) fader2Done = true;

    if (!fader1Done) digitalWrite(stepX, HIGH);
    if (!fader2Done) digitalWrite(stepY, HIGH);
    
    delayMicroseconds(speedDelay);

    if (!fader1Done) {
      digitalWrite(stepX, LOW);
      stepsFader1++;
    }
    if (!fader2Done) {
      digitalWrite(stepY, LOW);
      stepsFader2++;
    }
    
    delayMicroseconds(speedDelay);
  }

  int dist1 = round(stepsFader1 / stepsPerMM);
  int dist2 = round(stepsFader2 / stepsPerMM);

  Serial.print(dist1);
  Serial.print(",");
  Serial.println(dist2);

  // CRITICAL: Set the current absolute position to the top limit we just hit
  currentPosFader01 = dist1;
  currentPosFader02 = dist2;
  
  // (Optional) Reset gain knobs to 0 assuming they were manually zeroed
  currentPosGain01 = 0;
  currentPosGain02 = 0;
}

// ------------------------------------------------------------------
// PROGRAMME 2: Standard Operation (Absolute Positioning)
// ------------------------------------------------------------------
void parseAndExecute(String commandLine) {
  int commaIndex = commandLine.indexOf(',');
  if (commaIndex == -1) {
    return;
  }

  String device = commandLine.substring(0, commaIndex);
  float targetValue = commandLine.substring(commaIndex + 1).toFloat();

  // X Axis (Fader 01)
  if (device == "fader01") {
    if (targetValue != currentPosFader01) {
      
      // Calculate how far to move relative to current position
      float distanceToMove = targetValue - currentPosFader01;
      
      Serial.print("X Target: "); Serial.print(targetValue); Serial.println("mm");
      moveFader(stepX, dirX, distanceToMove);
      
      // Update memory to new absolute position
      currentPosFader01 = targetValue;
    }
  }
  
  // Y Axis (Fader 02)
  else if (device == "fader02") {
    if (targetValue != currentPosFader02) {
      
      float distanceToMove = targetValue - currentPosFader02;
      
      Serial.print("Y Target: "); Serial.print(targetValue); Serial.println("mm");
      moveFader(stepY, dirY, distanceToMove);
      
      currentPosFader02 = targetValue;
    }
  }
  
  // Z Axis (Gain 01)
  else if (device == "gain01") {
    if (targetValue != currentPosGain01) {
      
      float degreesToMove = targetValue - currentPosGain01;
      
      Serial.print("Z Target: "); Serial.print(targetValue); Serial.println(" deg");
      moveKnob(stepZ, dirZ, degreesToMove);
      
      currentPosGain01 = targetValue;
    }
  }
  
  // A Axis (Gain 02)
  else if (device == "gain02") {
    if (targetValue != currentPosGain02) {
      
      float degreesToMove = targetValue - currentPosGain02;
      
      Serial.print("A Target: "); Serial.print(targetValue); Serial.println(" deg");
      moveKnob(stepA, dirA, degreesToMove);
      
      currentPosGain02 = targetValue;
    }
  }
}

// ------------------------------------------------------------------
// Motor Execution Logic
// ------------------------------------------------------------------
void moveFader(int stepPin, int dirPin, float mm) {
  long stepsToTake = abs(mm) * stepsPerMM;
  digitalWrite(dirPin, (mm >= 0) ? HIGH : LOW);
  executeSteps(stepPin, stepsToTake);
}

void moveKnob(int stepPin, int dirPin, float degrees) {
  long stepsToTake = abs(degrees) * stepsPerDegree;
  digitalWrite(dirPin, (degrees >= 0) ? HIGH : LOW);
  executeSteps(stepPin, stepsToTake);
}

void executeSteps(int stepPin, long steps) {
  for (long i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speedDelay);

    digitalWrite(stepPin, LOW);
    delayMicroseconds(speedDelay);
  }
  Serial.println("Movement Complete");
}