#include <Encoder.h>

#if defined(ESP32)
#define DAC_BIT_WIDTH 8
#else
#define DAC_BIT_WIDTH 12
#endif

const int A_max = (1 << DAC_BIT_WIDTH) - 1;  // 4095 on 12 bit DAC
const int A_sustain = A_max * 0.75;
unsigned long T_attack = 100;         // Attack time in milliseconds
const unsigned long T_decay = 75;     // Decay time in milliseconds
const unsigned long T_sustain = 250;  // Sustain time in milliseconds
const unsigned long T_release = 250;  // Release time in milliseconds

// Time constant for exponential attack; Adjust to change shape of the curve.
const float attack_shape_factor = 5.0;

const float decay_shape_factor = 10.0;
const float tau_decay = T_decay / decay_shape_factor;

const float release_shape_factor = 1.0;  // low = linear, high = loggy
const float tau_release = T_release / release_shape_factor;

int envelopeValue = 0;
unsigned long startTime = 0;
int stage = 0;  // 0: Attack, 1: Decay, 2: Sustain, 3: Release, 4: Done

Encoder myEnc(5, 6);
const byte switchPin = 7;

long oldPosition = -999;

const byte buzzPin = 3;

void setup() {
  Serial.begin(9600);

#if defined(ESP32)
  delay(1000);  // wait for Serial on ESP32
#else
  analogWriteResolution(DAC_BIT_WIDTH);  // Max out DAC resolution
#endif

  // Initialize the envelope stage
  startTime = millis();
  stage = 0;

  Serial.println("Hello Envelope Generator");

  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(switchPin), handleSwitch, RISING);

  pinMode(buzzPin, OUTPUT);
  tone(buzzPin, 200);
}

void loop() {

  long newPosition = myEnc.read();
  if (newPosition != oldPosition) {
    // Turning knob right position goes down, but want attack to go up
    if (newPosition < oldPosition) {
      T_attack = T_attack + 25;

    } else if (newPosition > oldPosition) {
      if (T_attack >= 25) {
        T_attack = T_attack - 25;
      }
    }

    oldPosition = newPosition;
    Serial.print("Attack change: ");
    Serial.println(T_attack);
  }


  float tau_attack = T_attack / attack_shape_factor;



  // Example: Reset the envelope when a button is pressed
  //   if (digitalRead(2) == HIGH) {
  //     startTime = millis();
  //     stage = 0;
  //   }

  unsigned long currentTime = millis();
  unsigned long elapsedTime = currentTime - startTime;

  switch (stage) {
    case 0:  // Attack
      if (elapsedTime < T_attack) {
        envelopeValue = A_max * (1 - exp(-(elapsedTime / tau_attack)));
      } else {
        envelopeValue = A_max;
        startTime = currentTime;
        stage = 1;
      }
      break;

    case 1:  // Decay
      if (elapsedTime < T_decay) {
        envelopeValue = A_sustain + (A_max - A_sustain) * exp(-(elapsedTime / tau_decay));
      } else {
        envelopeValue = A_sustain;
        startTime = currentTime;
        stage = 2;
      }
      break;

    case 2:  // Sustain
      if (elapsedTime < T_sustain) {
        envelopeValue = A_sustain;
      } else {
        startTime = currentTime;
        stage = 3;
      }
      break;

    case 3:  // Release
      if (elapsedTime < T_release) {
        envelopeValue = A_sustain * exp(-(elapsedTime / tau_release));
      } else {
        envelopeValue = 0;
        stage = 4;
      }
      break;

    case 4:  // Done
      envelopeValue = 0;

      // TEMP: Restart the envelope
      startTime = currentTime;
      stage = 0;
      break;
  }
#if defined(ESP32)
  dacWrite(DAC1, envelopeValue);
#else
  analogWrite(DAC, envelopeValue);
#endif
}

void handleSwitch() {
  Serial.println("Switch pressed");
}