const int SolenoidA = 6; // Top valve terminal A
const int SolenoidB = 7; // Top valve terminal B
const int SolenoidC = 8; // Bottom valve terminal A
const int SolenoidD = 9; // Bottom valve terminal B

String inputString = "";     // A String to hold incoming data
bool stringComplete = false; // Whether the string is complete

void setup() {
  Serial.begin(9600);              

  pinMode(SolenoidA, OUTPUT); // Top valve
  pinMode(SolenoidB, OUTPUT);
  pinMode(SolenoidC, OUTPUT); // Bottom valve
  pinMode(SolenoidD, OUTPUT);

  // Default state: both valves open
  digitalWrite(SolenoidA, HIGH);
  digitalWrite(SolenoidB, LOW);
  delay(50);
  digitalWrite(SolenoidA, LOW);
  digitalWrite(SolenoidB, LOW);

  digitalWrite(SolenoidC, HIGH);
  digitalWrite(SolenoidD, LOW);
  delay(50);
  digitalWrite(SolenoidC, LOW);
  digitalWrite(SolenoidD, LOW);

  Serial.println("Ready. Type 'open top', 'close top', 'open bottom', or 'close bottom' to control the solenoid valves.");
}

void loop() {
  if (stringComplete) {
    inputString.trim(); // Remove trailing whitespace

    if (inputString.equalsIgnoreCase("open top")) {
      openTopValve();
    } else if (inputString.equalsIgnoreCase("close top")) {
      closeTopValve();
    } else if (inputString.equalsIgnoreCase("open bottom")) {
      openBottomValve();
    } else if (inputString.equalsIgnoreCase("close bottom")) {
      closeBottomValve();
    } else {
      Serial.println("Invalid command. Try 'open top', 'close top', 'open bottom', or 'close bottom'.");
    }

    inputString = "";
    stringComplete = false;
  }

  // Read incoming serial characters
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}

// Functions for top valve
void openTopValve() {
  digitalWrite(SolenoidA, HIGH);
  digitalWrite(SolenoidB, LOW);
  delay(50);
  digitalWrite(SolenoidA, LOW);
  digitalWrite(SolenoidB, LOW);
  Serial.println("Top valve opened.");
}

void closeTopValve() {
  digitalWrite(SolenoidA, LOW);
  digitalWrite(SolenoidB, HIGH);
  delay(50);
  digitalWrite(SolenoidA, LOW);
  digitalWrite(SolenoidB, LOW);
  Serial.println("Top valve closed.");
}

// Functions for bottom valve
void openBottomValve() {
  digitalWrite(SolenoidC, HIGH);
  digitalWrite(SolenoidD, LOW);
  delay(50);
  digitalWrite(SolenoidC, LOW);
  digitalWrite(SolenoidD, LOW);
  Serial.println("Bottom valve opened.");
}

void closeBottomValve() {
  digitalWrite(SolenoidC, LOW);
  digitalWrite(SolenoidD, HIGH);
  delay(50);
  digitalWrite(SolenoidC, LOW);
  digitalWrite(SolenoidD, LOW);
  Serial.println("Bottom valve closed.");
}
