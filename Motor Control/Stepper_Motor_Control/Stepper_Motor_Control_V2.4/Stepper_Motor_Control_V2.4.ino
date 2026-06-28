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

// Store CURRENT absolute positions of each motor
float currentPosFader01 = 0;
float currentPosFader02 = 0;
float currentPosGain01  = 0;
float currentPosGain02  = 0;

// --- System State ---
// 99 = Waiting, 0 = Manual Jog, 1 = Calibration, 2 = Normal Operation
int systemMode = 99;

// --- Jog Mode State (For Mode 0) ---
// 0 = None, 1 = Fader1, 2 = Fader2, 3 = Gain1, 4 = Gain2
int activeJogMotor = 0; 
// 1 = UP/CW, -1 = DOWN/CCW
int jogDirection = 0; 

void setup() {
  Serial.begin(9600);
  Serial.println("System Ready. Select a Mode:");
  Serial.println("Send '0' - Manual Jog Mode (Individual Motor Control)");
  Serial.println("Send '1' - Calibration Mode (Measure total distance)");
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
  // 1. Check for incoming Serial Commands
  if (Serial.available() > 0) {
    String commandLine = Serial.readStringUntil('\n');
    commandLine.trim();

    if (commandLine.length() > 0) {
      
      // -- Master Mode Switching --
      if (commandLine == "0") {
        systemMode = 0;
        activeJogMotor = 0; // Ensure motors are stopped when entering mode
        Serial.println("Mode 0 Active: Manual Jog.");
        Serial.println("Commands: 1u, 1d, 2u, 2d, 1cw, 1ccw, 2cw, 2ccw | 's' to STOP");
      }
      else if (commandLine == "1") {
        activeJogMotor = 0;
        runCalibration();
        systemMode = 99; // Return to waiting state after calibration
        Serial.println("Calibration Complete. Send '2' to begin operation.");
      } 
      else if (commandLine == "2") {
        systemMode = 2;
        activeJogMotor = 0;
        Serial.println("Mode 2 Active: Absolute Positioning. Format: <device>,<target_position>");
      } 
      
      // -- Mode 0: Continuous Jog Controls --
      else if (systemMode == 0) {
        if (commandLine == "s") {
          activeJogMotor = 0;
          Serial.println("Jog STOPPED.");
        } 
        else if (commandLine == "1u") { activeJogMotor = 1; jogDirection = 1; digitalWrite(dirX, HIGH); Serial.println("Fader 1 Jogging UP..."); }
        else if (commandLine == "1d") { activeJogMotor = 1; jogDirection = -1; digitalWrite(dirX, LOW); Serial.println("Fader 1 Jogging DOWN..."); }
        
        else if (commandLine == "2u") { activeJogMotor = 2; jogDirection = 1; digitalWrite(dirY, HIGH); Serial.println("Fader 2 Jogging UP..."); }
        else if (commandLine == "2d") { activeJogMotor = 2; jogDirection = -1; digitalWrite(dirY, LOW); Serial.println("Fader 2 Jogging DOWN..."); }
        
        else if (commandLine == "1cw") { activeJogMotor = 3; jogDirection = 1; digitalWrite(dirZ, HIGH); Serial.println("Gain 1 Jogging CW..."); }
        else if (commandLine == "1ccw") { activeJogMotor = 3; jogDirection = -1; digitalWrite(dirZ, LOW); Serial.println("Gain 1 Jogging CCW..."); }
        
        else if (commandLine == "2cw") { activeJogMotor = 4; jogDirection = 1; digitalWrite(dirA, HIGH); Serial.println("Gain 2 Jogging CW..."); }
        else if (commandLine == "2ccw") { activeJogMotor = 4; jogDirection = -1; digitalWrite(dirA, LOW); Serial.println("Gain 2 Jogging CCW..."); }
        
        else {
          Serial.println("Invalid Jog Command. Use 1u, 1d, 2u, 2d, 1cw, 1ccw, 2cw, 2ccw, or s.");
        }
      }
      
      // -- Mode 2: Absolute Positioning Controls --
      else if (systemMode == 2) {
        parseAndExecute(commandLine);
      } 
      else {
        Serial.println("Error: Send '0', '1', or '2' first to select a mode.");
      }
    }
  }

  // 2. Non-Blocking Motor Execution (For Jog Mode)
  if (systemMode == 0 && activeJogMotor != 0) {
    
    int currentStepPin = 0;
    bool limitHit = false;

    // Determine which pin to pulse and check safety limits
    if (activeJogMotor == 1) {
      if (jogDirection == 1 && digitalRead(itrPin1) == HIGH) limitHit = true;
      currentStepPin = stepX;
    } 
    else if (activeJogMotor == 2) {
      if (jogDirection == 1 && digitalRead(itrPin2) == HIGH) limitHit = true;
      currentStepPin = stepY;
    } 
    else if (activeJogMotor == 3) {
      currentStepPin = stepZ;
    } 
    else if (activeJogMotor == 4) {
      currentStepPin = stepA;
    }

    // Execute the step or stop if limit is reached
    if (limitHit) {
      activeJogMotor = 0;
      Serial.println("Top limit reached. Jog STOPPED.");
    } 
    else {
      digitalWrite(currentStepPin, HIGH);
      delayMicroseconds(speedDelay);
      
      digitalWrite(currentStepPin, LOW);
      delayMicroseconds(speedDelay);
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

  float dist1 = stepsFader1 / stepsPerMM;
  float dist2 = stepsFader2 / stepsPerMM;

  Serial.print(dist1);
  Serial.print(",");
  Serial.println(dist2);

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

  if (device == "fader01") {
    if (targetValue != currentPosFader01) {
      float distanceToMove = targetValue - currentPosFader01;
      Serial.print("X Target: "); Serial.print(targetValue); Serial.println("mm");
      moveFader(stepX, dirX, distanceToMove);
      currentPosFader01 = targetValue;
    }
  }
  else if (device == "fader02") {
    if (targetValue != currentPosFader02) {
      float distanceToMove = targetValue - currentPosFader02;
      Serial.print("Y Target: "); Serial.print(targetValue); Serial.println("mm");
      moveFader(stepY, dirY, distanceToMove);
      currentPosFader02 = targetValue;
    }
  }
  else if (device == "gain01") {
    if (targetValue != currentPosGain01) {
      float degreesToMove = targetValue - currentPosGain01;
      Serial.print("Z Target: "); Serial.print(targetValue); Serial.println(" deg");
      moveKnob(stepZ, dirZ, degreesToMove);
      currentPosGain01 = targetValue;
    }
  }
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
// Motor Execution Logic (Used for Mode 2)
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