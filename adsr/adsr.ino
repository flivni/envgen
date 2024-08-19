#include "AdsrEnvelope.h"
#include "Display.h"
#include "EncoderHandler.h"
#include "AdcHandler.h"

// TODO: Uncomment to auto-start the envelope generator
// #define AUTO_START_ENVELOPE

#if defined(ESP32) && defined(DAC1)
// #define USE_ESP32_DAC 1
#endif
#define USE_ESP32_DAC 1

#if defined(ESP32)
#define DAC_BIT_WIDTH 8
#define DAC_PIN 25
#else
#define DAC_BIT_WIDTH 12
#define DAC_PIN 7
#endif

const int WHITE_LED_PIN = 21;
const int BLUE_LED_PIN = 22;

int envelopeValue = 0;
unsigned long startTime = 0;

uint16_t A_max = (1 << DAC_BIT_WIDTH) - 1;
AdsrEnvelope adsr(
    A_max, // 4095 on 12 bit DAC
    100, // T_attack
    75, // T_decay
    A_max * 0.75, //  A_sustain
    250 // T_release
    );
Display display(240, 135);
EncoderHandler encoderHandler(&adsr);
AdcHandler adcHandler;

void setup() {
  Serial.begin(115200);

#if defined(ESP32)
  #if !defined(USE_ESP32_DAC) || USE_ESP32_DAC == 0 // we'll use PWM for ESP32-S3 since it doesn't have a DAC
      ledcAttach(DAC_PIN, 10000, DAC_BIT_WIDTH);
  #endif
  delay(1000);  // wait for Serial on ESP32
#else
  analogWriteResolution(DAC_BIT_WIDTH);  // Max out DAC resolution
#endif

#ifdef AUTO_START_ENVELOPE
  double now = millis();
  adsr.setEnvelopeStartTime(now);
  adsr.triggerRelease(now + 500 + adsr.getAttackDurationMs() + adsr.getDecayDurationMs());
#endif

  Serial.println("Hello Envelope Generator");

  display.init();
  encoderHandler.registerOnEncoderChange(onEncoderChanged);
  adcHandler.registerOnAdcChange(onAdcChanged);
  encoderHandler.setup();

  draw();
  adcHandler.setup();

  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(WHITE_LED_PIN, LOW);
  digitalWrite(BLUE_LED_PIN, HIGH);
}

void loop() {
  unsigned long currentTime = millis();

#ifdef AUTO_START_ENVELOPE
  if(currentTime > adsr.getEnvelopeStartTime() + adsr.getEnvelopeDurationMs()) {
    adsr.setEnvelopeStartTime(currentTime);
  }
#endif

  envelopeValue = adsr.getEnvelopeValue(currentTime);

  encoderHandler.tick();


#if defined(ESP32)
  #if defined(USE_ESP32_DAC) && USE_ESP32_DAC == 1
      dacWrite(DAC_PIN, envelopeValue);
  #else
      ledcWrite(DAC_PIN, envelopeValue);
  #endif
#else
  analogWrite(DAC_PIN, envelopeValue);
#endif
    
  adcHandler.tick();
}

void onEncoderChanged() {
   Serial.println("encoder changed") ;
   draw();
   Serial.println("END encoder changed") ;
}

void draw() {
    AdsrEnvelope drawAdsr = adsr;
    drawAdsr.startEnvelope(0);
    drawAdsr.setSustainDurationMs(adsr.getReleaseDurationMs());
    Serial.println("before draw");
    display.draw(&drawAdsr, &encoderHandler);
    Serial.println("after draw");
}

void onAdcChanged() {
  if (adcHandler.isNoteOn()) {
    #ifndef AUTO_START_ENVELOPE
    Serial.println("startEnvelope");
    adsr.startEnvelope(millis());
    #endif
    digitalWrite(WHITE_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
  } else {
    #ifndef AUTO_START_ENVELOPE
    Serial.println("triggerRelease");
    adsr.triggerRelease(millis());
    #endif
    digitalWrite(WHITE_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
  }
}
