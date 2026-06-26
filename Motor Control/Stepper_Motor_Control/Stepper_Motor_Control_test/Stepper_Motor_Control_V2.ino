// --- CNC Shield V3 Pin Definitions ---
const int enPin = 8; // Shared Enable pin

// X-Axis (fader01)
const int stepX = 2;
const int dirX  = 5;

// Y-Axis (fader02)
const int stepY = 3;
const int dirY  = 6;

// Z-Axis (gain01)
const int stepZ = 4;
const int dirZ  = 7;

// A-Axis (gain02) - Using independent D12/D13 pins
const int stepA = 12;
const int dirA  = 13;

// --- Conversion Constants ---
// 2048 steps / 2mm pitch = 1024 steps per mm
const float stepsPerMM = 1024.0; 

// 2048 steps / 360 degrees = ~5.6888 steps per degree
const float stepsPerDegree = 5.6888; 

// Motor Speed (Microseconds delay between pulses. Lower = Faster)
const int speedDelay = 800; 

void setup() {
  // Initialize Serial Communication
  Serial.begin(9600);
  Serial.println("System Ready. Waiting for commands...");
  Serial.println("Format: <device>,<value> (e.g., fader01,12 or gain02,-10)");

  // Configure Motor Pins
  pinMode(enPin, OUTPUT);
  pinMode(stepX, OUTPUT); pinMode(dirX, OUTPUT);
  pinMode(stepY, OUTPUT); pinMode(dirY, OUTPUT);
  pinMode(stepZ, OUTPUT); pinMode(dirZ, OUTPUT);
  pinMode(stepA, OUTPUT); pinMode(dirA, OUTPUT);

  // Enable all motor drivers (Active LOW)
  digitalWrite(enPin, LOW);
}

void loop() {
  // Check if data is available in the Serial buffer
  if (Serial.available() > 0) {
    // Read the incoming string until a newline character
    String commandLine = Serial.readStringUntil('\n');
    commandLine.trim(); // Remove any stray whitespace or carriage returns (\r)

    if (commandLine.length() > 0) {
      parseAndExecute(commandLine);
    }
  }
}

// Function to split the command and route it to the right motor
void parseAndExecute(String commandLine) {
  int commaIndex = commandLine.indexOf(',');
  
  // If there is no comma, the command format is invalid
  if (commaIndex == -1) {
    Serial.println("Error: Invalid format. Use device,value");
    return;
  }

  // Split the string into device name and the numeric value
  String device = commandLine.substring(0, commaIndex);
  float value = commandLine.substring(commaIndex + 1).toFloat();

  // Route to the appropriate motor based on the device name
  if (device == "fader01") {
    Serial.print("Moving X (fader01): "); Serial.print(value); Serial.println(" mm");
    moveFader(stepX, dirX, value);
    
  } else if (device == "fader02") {
    Serial.print("Moving Y (fader02): "); Serial.print(value); Serial.println(" mm");
    moveFader(stepY, dirY, value);
    
  } else if (device == "gain01") {
    Serial.print("Rotating Z (gain01): "); Serial.print(value); Serial.println(" degrees");
    moveKnob(stepZ, dirZ, value);
    
  } else if (device == "gain02") {
    Serial.print("Rotating A (gain02): "); Serial.print(value); Serial.println(" degrees");
    moveKnob(stepA, dirA, value);
    
  } else {
    Serial.println("Error: Unknown device name.");
  }
}

// Logic for Faders (Linear Movement)
void moveFader(int stepPin, int dirPin, float mm) {
  long stepsToTake = abs(mm) * stepsPerMM;
  
  // Determine direction: Positive = UP (HIGH), Negative = DOWN (LOW)
  // Note: If your physical motor moves backward, swap HIGH and LOW here, 
  // or flip the 4-pin motor connector on the CNC shield 180 degrees.
  bool isUp = (mm >= 0);
  digitalWrite(dirPin, isUp ? HIGH : LOW);
  

  executeSteps(stepPin, stepsToTake);
}

// Logic for Knobs (Rotational Movement)
void moveKnob(int stepPin, int dirPin, float degrees) {
  long stepsToTake = abs(degrees) * stepsPerDegree;
  
  // Determine direction: Positive = CW (HIGH), Negative = CCW (LOW)
  bool isCW = (degrees >= 0);
  digitalWrite(dirPin, isCW ? HIGH : LOW);

  executeSteps(stepPin, stepsToTake);
}

// The core loop that actually pulses the driver to step the motor
void executeSteps(int stepPin, long steps) {
  for (long i = 0; i < steps; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(speedDelay);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(speedDelay);
  }
  Serial.println("Movement Complete.");
}