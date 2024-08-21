#ifndef ENCODER_HANDLER_H
#define ENCODER_HANDLER_H

#include <Arduino.h>
#include "AdsrEnvelope.h"

#if defined(ESP32)
#include <ESP32Encoder.h>
#else
#include <Encoder.h>
#endif

class EncoderHandler {
    public:
    #if defined(ESP32)
    const int ENCODER_A_PIN = 32;
    const int ENCODER_B_PIN = 33;
    const int ENCODER_BUTTON_PIN = 26;
    #else
    const int ENCODER_A_PIN = 1;
    const int ENCODER_B_PIN = 2;
    const int ENCODER_BUTTON_PIN = 3;
    #endif

    // NB: I don't know the right values for these constants; these need to be validated
    static constexpr double MAX_ATTACK_DURATION_MS = 5000;
    static constexpr double MAX_ADR_DURATION_MS = 5000;
    static constexpr double MIN_ADR_DURATION_MS = 400;

    using OnEncoderChanged = void(*)();
    EncoderHandler(AdsrEnvelope* adsr);
    void setup();

    enum State {
        ATTACK_DURATION,
        ATTACK_SHAPE,
        ENVELOPE_DURATION
    };

    inline State getState() { return encoderState; }
    void tick();
    void registerOnEncoderChange(OnEncoderChanged callback);
    inline State getEncoderState() { return encoderState; }

    private: 
    static EncoderHandler* instance; 
    bool _buttonPressPending = false;
    AdsrEnvelope* adsr;
    #if defined(ESP32)
        ESP32Encoder encoder;
    #else
        Encoder encoder;
    #endif
    State encoderState;
    OnEncoderChanged onEncoderChanged;
    long lastButtonClickTime = 0;

    void onPushButtonImpl();
    
    long encoderPosition = 0;

    #if defined(ESP32)
    static void IRAM_ATTR onPushButton();
    # else
    static void onPushButton();
    #endif
};

#endif