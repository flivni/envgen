#include "AdsrEnvelope.h"
#include "Display.h"
#include "EncoderHandler.h"
#include "AdcHandler.h"

// TODO: Uncomment to auto-start the envelope generator
// #define AUTO_START_ENVELOPE

#if defined(ESP32)
#define DAC_BIT_WIDTH 8
#else
#define DAC_BIT_WIDTH 12
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
    250, // T_sustain
    A_max * 0.75, //  A_sustain
    250 // T_release
    );
Display display(240, 135);
EncoderHandler encoderHandler(&adsr);
AdcHandler adcHandler;

void setup() {
  Serial.begin(115200);

#if defined(ESP32)
  delay(1000);  // wait for Serial on ESP32
#else
  analogWriteResolution(DAC_BIT_WIDTH);  // Max out DAC resolution
#endif

#ifdef AUTO_START_ENVELOPE
  adsr.setEnvelopeStartTime(millis());
#endif

  Serial.println("Hello Envelope Generator");

  display.init();
  encoderHandler.registerOnEncoderChange(onEncoderChanged);
  adcHandler.registerOnAdcChange(onAdcChanged);
  encoderHandler.setup();
  display.draw(&adsr, &encoderHandler);

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
  dacWrite(DAC1, envelopeValue);
#else
  analogWrite(DAC, envelopeValue);
#endif
    
  adcHandler.tick();
}

void onEncoderChanged() {
  display.draw(&adsr, &encoderHandler);
}

void onAdcChanged() {
  if (adcHandler.isNoteOn()) {
    #ifndef AUTO_START_ENVELOPE
    adsr.setEnvelopeStartTime(millis());
    #endif
    digitalWrite(WHITE_LED_PIN, HIGH);
    digitalWrite(BLUE_LED_PIN, LOW);
  } else {
    #ifndef AUTO_START_ENVELOPE
    adsr.setEnvelopeStartTime(0);
    #endif
    digitalWrite(WHITE_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, HIGH);
  }
}
