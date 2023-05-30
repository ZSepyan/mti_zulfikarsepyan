include libraries

define pins:
nema 17 step motor 4 5 6 7
PIR1 8
PIR2 9
PIR3 10
PIR4 11
URM trig 3
URM echo 2
blueLED 12
buzzer A0
redLED A1
IRrecv 13

define constants

void setup() {
  serial begin
  enable IR

  setup pin modes:
    PIR1,2,3,4 INPUT
    URM trig OUTPUT
    URM echo INPUT
    blueLED OUTPUT
    redLED OUTPUT
    buzzer OUTPUT

  sensor calibration

  set step motor speed = 100 //1 rotation needs 200
  random seed
}

void function to move step motor according to given angle ()
  stepmotor.step(angle * 1rotation)

void function to measure distance ()
  turn on URM trig
  measure duration till URM echo receive the bouced signal
  calculate distance = duration * 0.034/2

void function to produce melody 1 ()

void function to produce melody 2 ()

void loop() {
  check if IR read anything
    if it reads anyting than 0 (brief press remote button)
      turn on redLED
      play melody 2
    if it reads 0 (long press remote button)
      turn off redLED
      play melody 1
  IR.resume()
  
  if redLED on, return (means code does not continue and keep looping here)

  /*
    continue to conditional function that set angle to mote step motor.
    this is according to where people coming.
    front = north, right = east, back = south, left = west.
    the estimation is calculated by data read from 1 sensor, 
    or superposition of 3 sensors.
    example if PIR 1 (front) receives data, 
    meaning people coming from front (north).
    also, if other PIRs receive data, but PIR 3 (back) does not, 
    meaning people coming from !back = front (north).

    remember to also mention about what sensor high, 
    and what sensor low.
    so arduino will less likely misunderstand the condition.
  
    melody 1 then will be played if the incoming people is too close.
  */

  //from NE
  if PIR 1 & 2 high, others low
    angle = 0.125
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1
  
  //from N
  if PIR 1 high & others low, or PIR 3 low & others high
    angle = 0
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1
  
  //from NW
  if PIR 1 & 4 high, others low
    angle = -0.125
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1

  //from W
  if PIR 4 high & others low, or PIR 2 low & others high
    angle = -0.25
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1

  //from SW
  if PIR 3 & 4 high, others low
    angle = -0.375
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1

  //from S
  if PIR 3 high & others low, or PIR 1 low & others high
    angle = 0.5
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1
  
  //from SE
  if PIR 1 high & others low, or PIR 3 low & others high
    angle = 0.375
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1
  
  //from E
  if PIR 2 high & others low, or PIR 4 low & others high
    angle = 0.25
    move step motor ()
    while 2s
      measure distance
      if too close
        play melody 1

  else
    blink blueLED

  bring back step motor to original position (facing north)
    angle = -angle
    move step motor ()
}
