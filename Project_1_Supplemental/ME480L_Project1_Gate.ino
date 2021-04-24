#include <LiquidCrystal.h> //include LCD library

// Pin numbers for LDC display
const int lcd_rs_pin = 14;
const int lcd_en_pin = 15;
const int lcd_db4_pin = 16;
const int lcd_db5_pin = 17;
const int lcd_db6_pin = 18;
const int lcd_db7_pin = 19;

// Initialize the library with the numbers of the interface pins
LiquidCrystal LCD(lcd_rs_pin, lcd_en_pin, lcd_db4_pin, lcd_db5_pin, lcd_db6_pin, lcd_db7_pin);


//Standard pins for the program
const int mot2DirAPin = 6;
const int mot2DirBPin = 7;
const int mot2PWMPin = 8;
const int btn1Pin = 29;
const int btn3Pin = 33;
const int btn4Pin = 35;
const int relayPin = 37;
const int greenLEDPin = 41;
const int redLED1Pin = 39;
const int blueLEDPin = 43;
const int redLED2Pin = 45;
const int edPin = A4;
const int gateDownPin = 9;
const int gateUpPin = 10;
const int EStopPin = 4;

//global variables
//State variables
bool WAIT = false;
bool OPEN = false;
bool ARMED = false;
bool TIMING1 = false;
bool TIMING2 = false;
bool CLOSE = true; //initialize the system to WAIT mode

//+++++++++++ Timer variables ++++++++++
unsigned long timer1DurationFar = 3000;  //time (ms) from start when gate will open (FAR)
unsigned long timer2DurationFar = 2000;  //time (ms) gate will remain open (FAR)
unsigned long timer1DurationNear = 2000; //time (ms) from start the gate will open (NEAR) 
unsigned long timer2DurationNear = 800;  //time (ms) gate will remain open (NEAR)
//++++++++++++++++++++++++++++++++++++++

unsigned long timerTarget = 0;
bool timerNear = true;

//Blink variables
unsigned long blinkTarget = 0;
unsigned long blinkDuration = 250;
bool blinkState = LOW;

//Motor control variables
int motorUpPWMCounts = 130;
int motorDownPWMCounts = 100;
int motorUpDirA = HIGH;
int motorUpDirB = LOW;
int motorDownDirA = LOW;
int motorDownDirB = HIGH;

//Emitter dector variables
int edThreshold = 100;

//choose debug output
int debugPrint = 0; //0 - standard, 1 - transitions and states

//Other global variables
bool btn1DownOld = false;
bool btn2DownOld = false;
bool btn3DownOld = false;
bool btn4DownOld = false;

void setup() {
  // setup the LCD's number of columns and rows:
  LCD.begin(8, 2);

  // setup the i/o pinMode
  pinMode(mot2DirAPin,OUTPUT);
  pinMode(mot2DirBPin,OUTPUT);
  pinMode(mot2PWMPin,OUTPUT);
  pinMode(btn1Pin,INPUT_PULLUP);
  pinMode(btn3Pin,INPUT_PULLUP);
  pinMode(btn4Pin,INPUT_PULLUP);
  pinMode(relayPin,OUTPUT);
  pinMode(greenLEDPin,OUTPUT);
  pinMode(redLED1Pin,OUTPUT);
  pinMode(blueLEDPin,OUTPUT);
  pinMode(redLED2Pin,OUTPUT);
  pinMode(btn1Pin,INPUT_PULLUP);
  pinMode(gateDownPin, INPUT_PULLUP);
  pinMode(gateUpPin, INPUT_PULLUP); 
  pinMode(EStopPin,INPUT_PULLUP);
  
  //make sure the motor is off
  digitalWrite(relayPin,LOW);
  digitalWrite(mot2DirAPin, LOW);
  digitalWrite(mot2DirBPin, LOW);
  analogWrite(mot2PWMPin, 0);

  //clear the LCD screen
  LCD.clear();
  LCD.print("PROJECT ");
  LCD.setCursor(0,1);
  LCD.print("   1    ");
  //turn on the serial communications
  Serial.begin(115200);
  delay(1000);
  LCD.clear();
  
  LCD.setCursor(0,1);
  if (timerNear){
    LCD.print("  NEAR  ");
  }
  else {
    LCD.print("  FAR   ");    
  }
}

void loop() {
  //Set the relay with EStop
  if (digitalRead(EStopPin)==HIGH){
    digitalWrite(relayPin,HIGH);
  }
  else {
    digitalWrite(relayPin,LOW);    
  }

  //update blink flag
  if (blinkTarget < millis()){
    blinkTarget = millis()+blinkDuration;
    if (blinkState==LOW) blinkState = HIGH;
    else blinkState = LOW;
  }

  //is the timer done
  bool timerDone (millis()>timerTarget);
  
  //Block 1 - inputs ++++++++++++++++++++++++++++++++++++++++++
  //check buttons for presses
  bool btn1Press = false;
  bool btn1Down = (digitalRead(btn1Pin)==LOW);
  if (btn1Down && !btn1DownOld){
    btn1Press = true;
  }
  btn1DownOld = btn1Down;
  
  bool btn3Press = false;
  bool btn3Down = (digitalRead(btn3Pin)==LOW);
  if (btn3Down && !btn3DownOld){
    btn3Press = true;
  }
  btn3DownOld = btn3Down;
  
  bool btn4Press = false;
  bool btn4Down = (digitalRead(btn4Pin)==LOW);
  if (btn4Down && !btn4DownOld){
    btn4Press = true;
  }
  btn4DownOld = btn4Down;
  

  //get the state of the emitter/dector
  bool edBlocked = false;
  if (analogRead(edPin)>edThreshold){
    edBlocked = true;
    digitalWrite(blueLEDPin, HIGH);
  }
  else {
    digitalWrite(blueLEDPin, LOW);    
  }

  //get the gate limit states
  bool gateUp = (digitalRead(gateUpPin)==LOW);
  bool gateDown = (digitalRead(gateDownPin)==LOW);
  

  //Block 2 - transitions ++++++++++++++++++++++++++++++++++++++++++
  bool WAIT_ARMED      = WAIT && btn1Press;
  bool WAIT_WAIT       = WAIT && !btn1Press;
  bool ARMED_TIMING1   = ARMED && edBlocked;
  bool ARMED_ARMED     = ARMED && !edBlocked;
  bool TIMING1_OPEN    = TIMING1 && timerDone;
  bool TIMING1_TIMING1 = TIMING1 && !timerDone;
  bool OPEN_TIMING2    = OPEN && gateUp;
  bool OPEN_OPEN       = OPEN && !gateUp;
  bool TIMING2_CLOSE   = TIMING2 && timerDone;
  bool TIMING2_TIMING2 = TIMING2 && !timerDone;
  bool CLOSE_WAIT      = CLOSE && gateDown;
  bool CLOSE_CLOSE     = CLOSE && !gateDown;


  //Block 3 - Set the states +++++++++++++++++++++++++++++++++++++++++++++++
  WAIT    = WAIT_WAIT || CLOSE_WAIT;
  ARMED   = ARMED_ARMED || WAIT_ARMED;
  TIMING1 = TIMING1_TIMING1 || ARMED_TIMING1;
  OPEN    = OPEN_OPEN || TIMING1_OPEN;
  TIMING2 = TIMING2_TIMING2 || OPEN_TIMING2;
  CLOSE   = CLOSE_CLOSE || TIMING2_CLOSE;

  if (debugPrint == 1){ //print out transition and state values
    Serial.print(WAIT_ARMED);
    Serial.print(WAIT_WAIT);
    Serial.print(ARMED_TIMING1);
    Serial.print(ARMED_ARMED);
    Serial.print(TIMING1_OPEN);
    Serial.print(TIMING1_TIMING1);
    Serial.print(OPEN_TIMING2);
    Serial.print(OPEN_OPEN);
    Serial.print(TIMING2_CLOSE);
    Serial.print(TIMING2_TIMING2);
    Serial.print(CLOSE_WAIT);
    Serial.print(CLOSE_CLOSE);
    Serial.print("/t");
    Serial.print(WAIT);
    Serial.print(OPEN);
    Serial.print(ARMED);
    Serial.print(TIMING1);
    Serial.print(OPEN);
    Serial.print(TIMING2);
    Serial.println(CLOSE);
  }

  //Block 4 - Do state actions and any clean up +++++++++++++++++++++++++++++++++++++++++++++++
  LCD.setCursor(0,0);
  
  if (WAIT){
    LCD.print("  WAIT  ");
    digitalWrite(greenLEDPin,HIGH);
    digitalWrite(redLED1Pin, LOW); 
        
    digitalWrite(mot2DirAPin, LOW);
    digitalWrite(mot2DirBPin, LOW);
    analogWrite(mot2PWMPin, 0);
    
    if (btn3Press){
      LCD.setCursor(0,1);
      LCD.print("  NEAR  ");
      timerNear = true;
    }
    if (btn4Press){
      LCD.setCursor(0,1);
      LCD.print("  FAR   ");
      timerNear = false;
    }
  }
  else{
    digitalWrite(greenLEDPin,LOW);
  }


  if (ARMED){
    LCD.print("  ARMED ");
    digitalWrite(redLED1Pin, blinkState);

    digitalWrite(mot2DirAPin, LOW);
    digitalWrite(mot2DirBPin, LOW);
    analogWrite(mot2PWMPin, 0);
  }
  
  if (TIMING1){
    LCD.print(" TIMING1");    
    digitalWrite(redLED1Pin, HIGH);
    if (ARMED_TIMING1){
      if (timerNear){
        timerTarget = millis()+timer1DurationNear;
      }
      else{
        timerTarget = millis()+timer1DurationFar;        
      }
    }
  }

  if (OPEN){
    LCD.print("  OPEN  ");  
    digitalWrite(mot2DirAPin, motorUpDirA);
    digitalWrite(mot2DirBPin, motorUpDirB);
    analogWrite(mot2PWMPin, motorUpPWMCounts);
    if (TIMING1_OPEN){
      if (timerNear){
        timerTarget = millis()+timer2DurationNear;
      }
      else{
        timerTarget = millis()+timer2DurationFar;        
      }
    }    
  }

  if (TIMING2){
    LCD.print(" TIMING2");    
    digitalWrite(redLED1Pin, HIGH);
    analogWrite(mot2PWMPin, 0);

  }
  
  if (CLOSE){
    LCD.print("  CLOSE ");  
    digitalWrite(mot2DirAPin, motorDownDirA);
    digitalWrite(mot2DirBPin, motorDownDirB);
    analogWrite(mot2PWMPin, motorDownPWMCounts);
  }

}