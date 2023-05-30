// this is pseudocode for Arturito
// author: Zulfikar Sepyan

// pins 12, 11, 6, 7, 8, 9 for LCD
// pin A0 for moisture sensor
// pin 2 for buzzer
// pin 3 for PIR
// pin 13 for blue led
// pin A2 for green led
// pin A4 for red led
// pin 4 and 5 for URM
// pin A1 for photoresistor

// create library for various greeting messages

void setup() {
  // initialize the machine
  // define input & output
  // set randomSeed
  // calibrate sensor
}

void loop() {
  // map moisture, ambience light, and distance reads
  // set all led off
  // if surrounding is dim, turned on yellow led. else, do nothing.
  
  // if moisture is low:
    // turn on red led
    // show water me message
    // only if there is anyone around
      // play R2D2 sound
      // vary the sound according to ambience light:
        // bright --> high pitch
        // dim --> low pitch
      // vary the delay according to distance with anyone around
        // near --> low delay / chattier
        // far --> long delay / less chatty

  // if moisture is high:
    // turn on green led
    // only if there is anyone around
      // if ambience light is bright
        // show various greeting message
        // play happy tone
      // if ambience light is dim
        // show awkward message
        // play sad tone

  // if there is sudden peak in moisture:
    // show thank you message
}
