// Libraries
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Wire.h>
#include <SparkFun_MMA8452Q.h>

#define FASTLED_INTERNAL
#include <FastLED.h>

// Bluetooth setup
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristic_2 = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
int pChar2_value_int = 0;
int old_pChar2_value_int = 0;

#define SERVICE_UUID        "1f646428-d6fb-4a51-a3b0-a1fb50573552"
#define CHAR1_UUID          "a91af3cf-b5fc-46b3-8fde-747656f5d590"
#define CHAR2_UUID          "9d091b6b-37ff-4c10-89e3-77b1799d0d49"

int connectMillis = 0;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    connectMillis = millis();
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class CharacteristicCallBack: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) override { 
    std::string pChar2_value_stdstr = pChar->getValue();
    String pChar2_value_string = String(pChar2_value_stdstr.c_str());
    pChar2_value_int = pChar2_value_string.toInt();
    Serial.println("pChar2: " + String(pChar2_value_int)); 
  }
};

// LED setup
FASTLED_USING_NAMESPACE
#define LED_TYPE WS2812
#define DATA_PIN 12 // Change to your LED pin
#define NUM_LEDS 24 // Change to match your LED setup
#define BRIGHTNESS 96 // Max LED brightness
#define FRAMES_PER_SECOND 120
#define LED_FADE_SPEED 20 // LED fade speed
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

// Accelerometer setup
#define SLEEP_DELAY 5000 // Delay for sleep mode check
#define ACCEL_THRESHOLD 2000 // Threshold for strong shaking
MMA8452Q accel;

typedef enum {
  SLEEP,
  STANDBY,
  CONCERT
} Mode;

Mode currentMode = SLEEP;
unsigned long lastModeChange = 0;
bool lockoutMode = false;

float movingAcceleration = 0;
float old_movingAcceleration = 0;

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHAR1_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );                   

  pCharacteristic_2 = pService->createCharacteristic(
                      CHAR2_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  
                    );  

  // Create a BLE Descriptor
  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue("A very interesting variable");
  pCharacteristic->addDescriptor(pDescr);
  
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  
  // Add all Descriptors here
  pCharacteristic->addDescriptor(pBLE2902);
  pCharacteristic_2->addDescriptor(new BLE2902());
  
  // After defining the desriptors, set the callback functions
  pCharacteristic_2->setCallbacks(new CharacteristicCallBack());
  
  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

  // Initialize LED
  delay(3000); // 3 second delay for recovery
  
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // Initialize Accelerometer
  Serial.println("MMA8452Q Test Code!");
  accel.init(SCALE_2G, ODR_1);
}

// LED mode
  typedef void (*SimplePatternList[])();
  SimplePatternList gPatterns = {juggle, confetti, rainbow, rainbowWithGlitter, bpm, sinelon, whiteFadeInOut, white, black, EXO};

  uint8_t gCurrentPatternNumber = 0; 
  uint8_t gHue = 0;
//

void loop() {
  // notify changed value
  if (deviceConnected) {
    pCharacteristic->setValue(value);
    pCharacteristic->notify();
    if (movingAcceleration > ACCEL_THRESHOLD && ((old_movingAcceleration - movingAcceleration) > 200 || (movingAcceleration - old_movingAcceleration) > 200)) {
      value++;
      old_movingAcceleration = movingAcceleration;
    }
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  // Main code
  if (accel.available()) {
    checkAccelerometer();
  } 
  runLED();
}

// LED array
  #define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))
//

void checkAccelerometer() {
  // Read accelerometer data
  accel.read();
  byte pl = accel.readPL();

  // Interpret accelerometer data
  shakeDetection();
  
  // Check for strong shaking to change modes
  if (currentMode != CONCERT && movingAcceleration > ACCEL_THRESHOLD) {
    switchMode();
  }

  // If in concert mode, check for lockout (laying flat for 5 seconds)
  if (currentMode == CONCERT) {
    if (pl == LOCKOUT) { // accel.readPL() = 4 = LOCKOUT by default
      if (!lockoutMode) {
        lockoutMode = true;
        lastModeChange = millis();
      } else if (millis() - lastModeChange >= SLEEP_DELAY) {
        switchMode2();
      }
    } else {
      lockoutMode = false;
    }
  }
}

void shakeDetection() {
  movingAcceleration = sqrt(accel.x*accel.x + accel.y*accel.y + accel.z*accel.z);
  Serial.println(movingAcceleration);
}

void runLED() {
  switch (currentMode) {
    case SLEEP:
      // Smoothly turn off LED
      gPatterns[8]();
      Serial.println("led off");
      break;
    case STANDBY:
      // Breathing mode - fade LED from black to white and back smoothly
      gPatterns[5]();
      Serial.println("EXO");
      break;
    case CONCERT:
      // Rainbow color with glitter and hue changing every 20ms
      gPatterns[pChar2_value_int]();
      switch (pChar2_value_int) {
        case 0: Serial.println("led juggle"); break;
        case 1: Serial.println("led confetti"); break;
        case 2: Serial.println("led rainbow"); break;
        case 3: Serial.println("led rainbowWithGlitter"); break;
        case 4: Serial.println("led bpm"); break;
        case 5: Serial.println("led sinelon"); break;
        case 6: Serial.println("led whiteFadeInOut"); break;
        case 7: Serial.println("led white"); break;
      }
      break;
  }

  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  EVERY_N_MILLISECONDS(20) { 
    gHue++; 
  }
}

void switchMode() {
  switch (currentMode) {
    case SLEEP:
      currentMode = STANDBY;
      accel.setDataRate(ODR_6); // Increase data rate
      Serial.println("Switched to STANDBY mode");
      if(connectMillis > 0 && millis() - connectMillis < 120000) {
        EXO();
      }
      break;
    case STANDBY:
      currentMode = CONCERT;
      Serial.println("Switched to CONCERT mode");
      break;
  }
  lastModeChange = millis();
  delay(1000);
}

void switchMode2() {
  switch (currentMode) {
    case CONCERT:
    currentMode = SLEEP;
    accel.setDataRate(ODR_1); // Decrease data rate
    Serial.println("Switched to SLEEP mode");
    break;
  }
  lastModeChange = millis();
  delay(1000);
}

void black() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

void white() {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White;
  }
}

void sinelon() {
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  uint8_t dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void whiteFadeInOut() {
  for(int i = 0; i < 24; i++) {
    leds[i] = CRGB::WhiteSmoke;
    leds[i].maximizeBrightness(beatsin8(30, 0, 255));
  }
}

void EXO() {
  int fadeAmountExo = 5;
  int brightnessExo = 0;
  while(brightnessExo > -1) {
    for(int i = 14; i < 21; i++) {
      leds[i] = CRGB::WhiteSmoke;
      leds[i].maximizeBrightness(brightnessExo);
    }
    FastLED.show();
    brightnessExo = brightnessExo + fadeAmountExo;
    if(brightnessExo == 255) {
      fadeAmountExo = -fadeAmountExo*3;
    }
    delay(20);
  }

  delay(100);
  fadeAmountExo = 5;
  brightnessExo = 0;
  while(brightnessExo > -1) {
    for(int i = 21; i < 24; i++) {
      leds[i] = CRGB::WhiteSmoke;
      leds[i].maximizeBrightness(brightnessExo);
    }
    for(int i = 9; i < 14; i++) {
      leds[i] = CRGB::WhiteSmoke;
      leds[i].maximizeBrightness(brightnessExo);
    }
    for(int i = 0; i < 2; i++) {
      leds[i] = CRGB::WhiteSmoke;
      leds[i].maximizeBrightness(brightnessExo);
    }
    FastLED.show();
    brightnessExo = brightnessExo + fadeAmountExo;
    if(brightnessExo == 255) {
      fadeAmountExo = -fadeAmountExo*3;
    }
    delay(20);
  }

  delay(100);
  fadeAmountExo = 5;
  brightnessExo = 0;
  while(brightnessExo > -1) {
    for(int i = 2; i < 9; i++) {
      leds[i] = CRGB::WhiteSmoke;
      leds[i].maximizeBrightness(brightnessExo);
    }
    FastLED.show();
    brightnessExo = brightnessExo + fadeAmountExo;
    if(brightnessExo == 255) {
      fadeAmountExo = -fadeAmountExo*3;
    }
    delay(20);
  }

  delay(20);
  fadeAmountExo = 5;
  brightnessExo = 0;
  while(brightnessExo > -1) {
    for(int i = 0; i < 24; i++) {
      leds[i] = CRGB::WhiteSmoke;
      leds[i].maximizeBrightness(brightnessExo);
    }
    FastLED.show();
    brightnessExo = brightnessExo + fadeAmountExo;
    if(brightnessExo == 255) {
      fadeAmountExo = -fadeAmountExo*3;
    }
    delay(20);
  }
}
