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
const int speedDelay = 800;

// Store CURRENT absolute positions of each motor (Used in Mode 2)
float currentPosFader01 = 0;
float currentPosFader02 = 0;
float currentPosGain01  = 0;
float currentPosGain02  = 0;

// --- System State ---
// 99 = Waiting, 0 = Manual Jog (Relative), 1 = Calibration, 2 = Normal Operation (Absolute)
int systemMode = 99;

void setup() {
  Serial.begin(9600);
  Serial.println("System Ready. Select a Mode:");
  Serial.println("Send '0' - Manual Jog Mode (Move to bottom)");
  Serial.println("Send '1' - Calibration Mode (Measure distance)");
  Serial.println("Send '2' - Standard Operation (Absolute positioning)");

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
      if (commandLine == "0") {
        systemMode = 0;
        Serial.println("Mode 0 Active: Manual Jog. Format: <device>,<relative_distance> (e.g., fader01,-10)");
      }
      else if (commandLine == "1") {
        runCalibration();
        systemMode = 99; // Return to waiting state after calibration
        Serial.println("Calibration Complete. Send '2' to begin operation.");
      } 
      else if (commandLine == "2") {
        systemMode = 2;
        Serial.println("Mode 2 Active: Absolute Positioning. Format: <device>,<target_position>");
      } 
      
      // Execution Routing based on Mode
      else if (systemMode == 0) {
        parseAndExecuteRelative(commandLine); // Programme 0
      }
      else if (systemMode == 2) {
        parseAndExecute(commandLine);         // Programme 2
      } 
      else {
        Serial.println("Error: Send '0', '1', or '2' first to select a mode.");
      }
    }
  }
}

// ------------------------------------------------------------------
// PROGRAMME 0: Manual Jog Mode (Relative Positioning)
// ------------------------------------------------------------------
void parseAndExecuteRelative(String commandLine) {
  int commaIndex = commandLine.indexOf(',');
  if (commaIndex == -1) {
    Serial.println("Invalid format. Use: device,value");
    return;
  }

  String device = commandLine.substring(0, commaIndex);
  float moveValue = commandLine.substring(commaIndex + 1).toFloat();

  if (device == "fader01") {
    Serial.print("X Jogging: "); Serial.print(moveValue); Serial.println("mm");
    moveFader(stepX, dirX, moveValue);
  }
  else if (device == "fader02") {
    Serial.print("Y Jogging: "); Serial.print(moveValue); Serial.println("mm");
    moveFader(stepY, dirY, moveValue);
  }
  else if (device == "gain01") {
    Serial.print("Z Jogging: "); Serial.print(moveValue); Serial.println(" deg");
    moveKnob(stepZ, dirZ, moveValue);
  }
  else if (device == "gain02") {
    Serial.print("A Jogging: "); Serial.print(moveValue); Serial.println(" deg");
    moveKnob(stepA, dirA, moveValue);
  }
  else {
    Serial.println("Unknown device.");
  }
}

// ------------------------------------------------------------------
// PROGRAMME 1: Calibration & Distance Measurement
// ------------------------------------------------------------------
void runCalibration() {
  Serial.println("Calibrating... Moving faders to top sensors.");
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
  
  currentPosGain01 = 0;
  currentPosGain02 = 0;
}

// ------------------------------------------------------------------
// PROGRAMME 2: Standard Operation (Absolute Positioning)
// ------------------------------------------------------------------
void parseAndExecute(String commandLine) {
  int commaIndex = commandLine.indexOf(',');
  if (commaIndex == -1) {
    Serial.println("Invalid format. Use: device,value");
    return;
  }

  String device = commandLine.substring(0, commaIndex);
  float targetValue = commandLine.substring(commaIndex + 1).toFloat();

  // X Axis (Fader 01)
  if (device == "fader01") {
    if (targetValue != currentPosFader01) {
      float distanceToMove = targetValue - currentPosFader01;
      Serial.print("X Target: "); Serial.print(targetValue); Serial.println("mm");
      moveFader(stepX, dirX, distanceToMove);
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
  else {
    Serial.println("Unknown device.");
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