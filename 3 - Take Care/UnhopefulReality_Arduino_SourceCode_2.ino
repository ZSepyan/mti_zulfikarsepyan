/*
  (un)Hopeful Reality [2nd Arduino] - Make Things Interactive
  Code by Big Baby group, RMIT MDIT students
  Year 2023
  Youtube: 

  Description:
    This is the code for the second Arduino on the wall.
    It communicates with the second Arduino on the wearable using IR.
    It uses IR receiver & transmitter, LCD, buzzer, button and LEDs.

    More details about this project is on the attached slide.
*/

#include <IRremote.hpp>
#include <LiquidCrystal.h> 

//define pins
  LiquidCrystal LCD(12, 11, 6, 7, 8, 9);
  #define button 3
  #define systemLED A2
  #define buzzer A3

  //IR
  #define receiver A0
  #define sender A1

  #define greenLED 4
  #define redLED 5
//

//define constants
  //IR
  const unsigned long IrHexValue = 0xC3FF3; //sender
  const unsigned long IrHexExpected = 0xFEA857; //receiver
//

// abort mission request variable
bool shouldSystemBeOff = false;

void setup() {
  // begin communication with serial monitor
  Serial.begin(9600);
  // initialize LCD communication
  LCD.begin(16, 2);
  // begin IR communication
  IrReceiver.begin(receiver);
  IrSender.begin(sender);

  // initialize other I/O
  pinMode(button, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(systemLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);
}

void loop() {
  // checking IR communication
  if (IrReceiver.decode()) {
    Serial.println("IR received ");
    Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);

    if (IrReceiver.decodedIRData.decodedRawData == IrHexExpected) {
      shouldSystemBeOff = !shouldSystemBeOff;
      sadTone();
    }
  }
  IrReceiver.resume();

  // checking for any abort mission request
  if (shouldSystemBeOff) {
    systemShutDown();
    LCD.clear();
    return;
  }
  else {
    digitalWrite(systemLED, HIGH);
  }

  // hello world on LCD
  LCD.setCursor(0,0);
  LCD.print("this is");
  LCD.setCursor(0,1);
  LCD.print("the finish line!");

  // checking for button press
  if (digitalRead(button) == HIGH) {
    buttonPressed();
  }
}

void systemShutDown() {
  // turn off light
  digitalWrite(systemLED, LOW);
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, LOW);
}

void buttonPressed() {
  // send IR signal to shut down 1st Arduino
  IrSender.sendNECRaw(IrHexValue, 32);
  LCD.clear();

  // play happy tone
  happyTone();

  // green light for a while
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  // LCD screen [Thank you.... for donating!]
  LCD.setCursor(0, 0);
  LCD.print("Thank you");
  for (int i = 0; i < 6; i++) {
    LCD.setCursor(10 + i, 0);
    LCD.print(".");
    delay(1000);
  }
  LCD.setCursor(0,1);
  LCD.print("for Donating!");
  
  // then red light
  digitalWrite(greenLED, LOW);
  digitalWrite(redLED, HIGH);

  delay(5000);
  LCD.clear();
  digitalWrite(redLED, LOW);

  // play sad tone
  sadTone();
}

void sadTone() {
  for (int i = 0; i < 1; i++){  
    tone(buzzer, 523.25); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 490.55); delay(200); noTone(buzzer); delay(100); 
    tone(buzzer, 436.04); delay(200); noTone(buzzer); delay(100);  
    tone(buzzer, 392.44); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 348.83); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 327.03); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 294.33); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 261.626); delay(200); noTone(buzzer); delay(100);     
  }
}

void happyTone() {
  for (int i = 0; i < 1; i++){
    tone(buzzer, 261.626); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 294.33); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 327.03); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 348.83); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 392.44); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 436.04); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 490.55); delay(200); noTone(buzzer); delay(100);
    tone(buzzer, 523.25); delay(200); noTone(buzzer); delay(100);        
  }
}
