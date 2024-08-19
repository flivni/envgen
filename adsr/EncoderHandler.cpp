#include "EncoderHandler.h"

EncoderHandler* EncoderHandler::instance = nullptr;  

EncoderHandler::EncoderHandler(AdsrEnvelope* adsr) : 
        adsr(adsr), 
#if !defined(ESP32)
        encoder(ENCODER_A_PIN, ENCODER_B_PIN), 
#endif
        encoderState(ATTACK_DURATION) {
    instance = this;
}

void EncoderHandler::setup() {
    pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);
#if defined(ESP32)
    encoder.attachHalfQuad(ENCODER_A_PIN, ENCODER_B_PIN);
#endif
    attachInterrupt(digitalPinToInterrupt(ENCODER_BUTTON_PIN), onPushButton, RISING);
}

void EncoderHandler::registerOnEncoderChange(void (*callback)()) {
    onEncoderChanged = callback;
}

void EncoderHandler::tick() {
#if defined(ESP32)
    long newEncoderPosition = encoder.getCount() / 2;
#else
    long newEncoderPosition = encoder.read() / 4;
#endif

    if (newEncoderPosition != encoderPosition) {
        Serial.print("Encoder position: ");
        Serial.println(newEncoderPosition);

        long encoderDelta = (newEncoderPosition - encoderPosition);
        encoderPosition = newEncoderPosition;
        double attackDurationMs;
        double attackShapeFactor;
        double maxAttackDurationMs = 5000;
        switch (encoderState) {
        case ATTACK_DURATION:
            attackDurationMs = adsr->getAttackDurationMs();
            Serial.print("Attack duration WAS: ");
            Serial.println(attackDurationMs);
            attackDurationMs += 50 * encoderDelta;
            if (attackDurationMs > maxAttackDurationMs) {
                attackDurationMs = maxAttackDurationMs;
            }
            if (attackDurationMs < 50) {
                attackDurationMs = 50;
            }
            adsr->setAttackDurationMs(attackDurationMs);
            Serial.print("Attack duration IS: ");
            Serial.println(attackDurationMs);
            break;
        case ATTACK_SHAPE:
            attackShapeFactor = adsr->getAttackShapeFactor();
            attackShapeFactor += 0.5 * encoderDelta;
            if (attackShapeFactor > 10) {
                attackShapeFactor = 10;
            } else if (attackShapeFactor < 4) {
                attackShapeFactor = 4;
            }
            adsr->setAttackShapeFactor(attackShapeFactor);
            Serial.print("Attack shape factor: ");
            Serial.println(attackShapeFactor);
            break;
        case ENVELOPE_DURATION:
            double adrSegmentsDurationMs = adsr->getAttackDurationMs() + adsr->getDecayDurationMs() + 
                adsr->getReleaseDurationMs();
            double adjustedAdrSegmentsDurationMs = adrSegmentsDurationMs + 50 * encoderDelta;
            
            if (adjustedAdrSegmentsDurationMs > 5000) {
                return;
            } else if (adjustedAdrSegmentsDurationMs < 400) {
                return;
            }

            double ratio = adjustedAdrSegmentsDurationMs / adrSegmentsDurationMs;
            adsr->setAttackDurationMs(adsr->getAttackDurationMs() * ratio);
            adsr->setdecayDurationMs(adsr->getDecayDurationMs() * ratio);  
            adsr->setReleaseDurationMs(adsr->getReleaseDurationMs() * ratio);
            break;
        }
        // END switch

        if (onEncoderChanged) {
            onEncoderChanged();
        }
   }

    if (_buttonPressPending) {
        _buttonPressPending = false;
        if (onEncoderChanged) {
            onEncoderChanged();
        }
    }
}

void EncoderHandler::onPushButtonImpl() {
    // NB: not okay to call Serial.println method on ESP32 here as it blocks.

    // simple debounce
    if (millis() - lastButtonClickTime < 100) {
        return;
    }
    lastButtonClickTime = millis();
    encoderState = (EncoderHandler::State)((encoderState + 1) % 3);

    _buttonPressPending = true;
}


#if defined(ESP32)
void IRAM_ATTR EncoderHandler::onPushButton() {
# else
void EncoderHandler::onPushButton() {
#endif
    if (instance) {
        instance->onPushButtonImpl();
    }
}