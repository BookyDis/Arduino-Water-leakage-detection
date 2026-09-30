#include <SoftwareSerial.h>
//sensor pins
// INT 01 and 02 supports interrupt
const byte topOutlet = 2;       
const byte bottomOutlet = 3;    
const byte topIntake = 4;
const byte bottomIntake = 5;
//assign pins to send signals to the H-bridge
//H-bridge signal to the top valve
const int valveTop_A = 6;
const int valveTop_B = 7;
//H-bridge signal to the bottom valve
const int valveBottom_A = 8;
const int valveBottom_B = 9;

//define RX and TX on the UNO R3
SoftwareSerial espSerial(10, 11); 
//variable to hold litre/hour results
unsigned int top_l_hour = 0;
unsigned int bottom_l_hour = 0;
unsigned int top_outlet_l_hour = 0;
unsigned int bottom_outlet_l_hour = 0;
//timing
unsigned long currentTime;
unsigned long cloopTime;
//flow pulse counters use to trigger calculation every second
volatile int top_flow_frequency = 0;
volatile int bottom_flow_frequency = 0;
unsigned int top_outlet_frequency = 0;
unsigned int bottom_outlet_frequency = 0;
//water leakage threshold for top and bottom of the pipes
int top_water_leakage_threshold = 250; //L/h
int bottom_water_leakage_threshold = 350;  //L/h
//to keep track of the valve's state
bool valveTopIsOpen = false;
bool valveBottomIsOpen = false;
//for edge detection on outlets
bool lastTopOutletState = LOW;
bool lastBottomOutletState = LOW;

//interrupt function or the ISP ( Interrupt service routine)
void flow_top() {
  top_flow_frequency++;
}

void flow_bottom() {
  bottom_flow_frequency++;
}

void setup() {

  //set pin modes
  pinMode(topIntake, INPUT_PULLUP);
  pinMode(bottomIntake, INPUT_PULLUP);
  pinMode(topOutlet, INPUT_PULLUP);
  pinMode(bottomOutlet, INPUT_PULLUP);

  //assign pins to send signals to the H-bridge
  pinMode(valveTop_A, OUTPUT);
  pinMode(valveTop_B, OUTPUT);
  pinMode(valveBottom_A, OUTPUT);
  pinMode(valveBottom_B, OUTPUT);

  
  //serial setup
  Serial.begin(9600);
  espSerial.begin(9600);

  digitalWrite(valveTop_A, HIGH); // Open valve
  digitalWrite(valveTop_B, LOW);
  delay(50);

  //attach interrupts to both intakes
  attachInterrupt(digitalPinToInterrupt(topOutlet), flow_top, RISING);
  attachInterrupt(digitalPinToInterrupt(bottomOutlet), flow_bottom, RISING);

  currentTime = millis();
  cloopTime = currentTime;
}

void loop() {
  currentTime = millis();

  //poll outlet pins to simulate pulse counting
  bool topOutletState = digitalRead(topOutlet);
  bool bottomOutletState = digitalRead(bottomOutlet);

  //edge detection for outlets
  if (topOutletState == HIGH && lastTopOutletState == LOW) {
    top_outlet_frequency++;
  }
  lastTopOutletState = topOutletState;

  if (bottomOutletState == HIGH && lastBottomOutletState == LOW) {
    bottom_outlet_frequency++;
  }
  lastBottomOutletState = bottomOutletState;

  if (currentTime - cloopTime >= 1000) { // Every second
    //used a formula to calculate flow rates
    top_l_hour = (top_flow_frequency * 60 / 7.5);
    bottom_l_hour = (bottom_flow_frequency * 60 / 7.5);
    top_outlet_l_hour = (top_outlet_frequency * 60 / 7.5);
    bottom_outlet_l_hour = (bottom_outlet_frequency * 60 / 7.5);

    //reset counters
    top_flow_frequency = 0;
    bottom_flow_frequency = 0;
    top_outlet_frequency = 0;
    bottom_outlet_frequency = 0;

    cloopTime = currentTime;

    //top valve closing and opening function
    if (top_outlet_l_hour <= top_water_leakage_threshold && valveTopIsOpen) {
      digitalWrite(valveTop_A, LOW); // Close valve
      digitalWrite(valveTop_B, HIGH);
      delay(50);
      valveTopIsOpen = false;
    } else if (top_outlet_l_hour > top_water_leakage_threshold && !valveTopIsOpen) {
      digitalWrite(valveTop_A, HIGH); // Open valve
      digitalWrite(valveTop_B, LOW);
      delay(50);
      valveTopIsOpen = true;
      
    }

    //bottom valve closing and opening function
    if (bottom_outlet_l_hour <= bottom_water_leakage_threshold && valveBottomIsOpen) {
      digitalWrite(valveBottom_A, LOW); // Close valve
      digitalWrite(valveBottom_B, HIGH);
      delay(50); 
      valveBottomIsOpen = false;
    } else if (bottom_outlet_l_hour > bottom_water_leakage_threshold && !valveBottomIsOpen) {
      digitalWrite(valveBottom_A, HIGH); // Open valve
      digitalWrite(valveBottom_B, LOW);
      delay(50);
      valveBottomIsOpen = true;
    }

    //print on the UNO R3 serial monitor for debugging
    Serial.println("---------- Water flow readings ---------");
    Serial.print("Top Outlet Flow Rate     : "); Serial.print(top_outlet_l_hour); Serial.println(" L/hour");
    Serial.print("Bottom Outlet Flow Rate  : "); Serial.print(bottom_outlet_l_hour); Serial.println(" L/hour");
    Serial.print("Top Intake Flow Rate     : "); Serial.print(top_l_hour); Serial.println(" L/hour");
    Serial.print("Bottom Intake Flow Rate  : "); Serial.print(bottom_l_hour); Serial.println(" L/hour");
    Serial.print("Top Valve Status         : "); Serial.println(valveTopIsOpen ? "OPEN" : "CLOSED");
    Serial.print("Bottom Valve Status      : "); Serial.println(valveBottomIsOpen ? "OPEN" : "CLOSED");
    Serial.println("----------------------------------------");
    //add newline for readability
    Serial.println(); 


    //print data to send to ESP32 Nano
    espSerial.print("Top Outlet: "); espSerial.print(top_outlet_l_hour); espSerial.println(" L/hour");
    espSerial.print("Bottom Outlet: "); espSerial.print(bottom_outlet_l_hour); espSerial.println(" L/hour");
    espSerial.print("Top Intake: "); espSerial.print(top_l_hour); espSerial.println(" L/hour");
    espSerial.print("Bottom Intake: "); espSerial.print(bottom_l_hour); espSerial.println(" L/hour");
    espSerial.print("Top valve: "); espSerial.println(valveTopIsOpen ? "OPEN" : "CLOSED");
    espSerial.print("Bottom valve: "); espSerial.println(valveBottomIsOpen ? "OPEN" : "CLOSED");

  }
}
