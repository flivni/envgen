#include "AdsrEnvelope.h"
#include "Display.h"
#include "EncoderHandler.h"

#if defined(ESP32) && defined(DAC1)
// #define USE_ESP32_DAC 1
#endif

#if defined(ESP32)
#define DAC_BIT_WIDTH 8
#define DAC_PIN 25
#else
#define DAC_BIT_WIDTH 12
#define DAC_PIN 7
#endif

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

  adsr.setEnvelopeStartTime(millis());

  Serial.println("Hello Envelope Generator");

  display.init();
  encoderHandler.registerOnEncoderChange(onEncoderChanged);
  encoderHandler.setup();
  display.draw(&adsr, &encoderHandler);
}

void loop() {
  unsigned long currentTime = millis();

  // For now, automatically trigger a new adsr envelope once the previous one has finished
  if(currentTime > adsr.getEnvelopeStartTime() + adsr.getEnvelopeDurationMs()) {
    adsr.setEnvelopeStartTime(currentTime);
  }

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
}

void onEncoderChanged() {
  display.draw(&adsr, &encoderHandler);
}
