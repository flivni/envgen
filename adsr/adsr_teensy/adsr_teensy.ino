#include <Encoder.h>
#include <Bounce2.h>

#include <Adafruit_MCP4728.h>
#include <Adafruit_MCP23X17.h>

#include <Wire.h>


Bounce2::Button button = Bounce2::Button();

Encoder myEnc(0, 1);
long oldPosition = -999;

// DAC
#define DAC_BIT_WIDTH 12
Adafruit_MCP4728 mcp_dac;

// Expander
#define LED_PIN 0  // MCP23XXX pin LED is attached to
#define BUTTON_PIN 1

Adafruit_MCP23X17 mcp_exp;

// ENVELOPE CONSTS

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

// SETUP
// SETUP
// SETUP
void setup() {
  // put your setup code here, to run once:
  button.attach(2, INPUT_PULLUP);
  button.interval(10);
  button.setPressedState(HIGH);

  // Try to initialize DACs
  // Use address 96, I2C0 pins 18/19
  if (!mcp_dac.begin(96, &Wire)) {
    Serial.println("Failed to find MCP4728 chip");
    while (1) {
      delay(10);
    }
  }

  // Try to initialize expander
  // Use address 32, I2C1 pins 18/19
  if (!mcp_exp.begin_I2C(32, &Wire1)) {
    Serial.println("Error.");
    while (1)
      ;
  }

  // Configure LED pin for output
  mcp_exp.pinMode(LED_PIN, OUTPUT);
  // Configure button pin for input
  mcp_exp.pinMode(BUTTON_PIN, INPUT_PULLUP);


  // Set DAC test values
  // mcp_dac.setChannelValue(MCP4728_CHANNEL_A, 4095);
  // mcp_dac.setChannelValue(MCP4728_CHANNEL_B, 2048);
  // mcp_dac.setChannelValue(MCP4728_CHANNEL_C, 1024);
  // mcp_dac.setChannelValue(MCP4728_CHANNEL_D, 4095);
}

// LOOP
// LOOP
// LOOP
void loop() {
  // Expander test
  mcp_exp.digitalWrite(LED_PIN, !mcp_exp.digitalRead(BUTTON_PIN));


  button.update();
  if (button.pressed()) {
    Serial.println("ass");
  }

  long newPosition = myEnc.read() / 4;





  if (newPosition != oldPosition) {
    if (newPosition < oldPosition) {
      T_attack = T_attack + 25;

    } else if (newPosition > oldPosition) {
      if (T_attack >= 25) {
        T_attack = T_attack - 25;
      }
    }

    oldPosition = newPosition;
    Serial.println(newPosition);
  }

  // GENERATE ENVELOPE
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
  mcp_dac.setChannelValue(MCP4728_CHANNEL_A, envelopeValue);
}
