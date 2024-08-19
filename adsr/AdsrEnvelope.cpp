#include <Arduino.h>
#include "AdsrEnvelope.h"

AdsrEnvelope::AdsrEnvelope(double envelopeMax, double attackDurationMs, double decayDurationMs,  double sustainMax, 
        double releaseDurationMs) {
    this->envelopeMax = envelopeMax;
    this->attackDurationMs = attackDurationMs;
    this->decayDurationMs = decayDurationMs;
    this->sustainDurationMs = MAX_TIME;
    this->sustainMax = sustainMax;
    this->releaseDurationMs = releaseDurationMs;

    this->attackShapeFactor = 5.0;
    this->decayShapeFactor = 10.0;
    this->releaseShapeFactor = 1.0;
}

void AdsrEnvelope::startEnvelope(double time) { 
    this->envelopeStartTime = time;
    this->sustainDurationMs = MAX_TIME;
}

void AdsrEnvelope::triggerRelease(double time) { 
    double sustainStartMs = this->envelopeStartTime + this->attackDurationMs + this->decayDurationMs;
    if (time < sustainStartMs) {
        this->sustainDurationMs = 0;
    } else {
        this->sustainDurationMs = time - sustainStartMs;
    }
}

double AdsrEnvelope::getEnvelopeValue(double time) {
    double envelopeValue = 0.0;
    double timeSinceStart = time - this->envelopeStartTime;
    if (timeSinceStart < this->attackDurationMs) {
        // attack phase
        double phaseTime = timeSinceStart;
        if (attackShapeFactor < 5.0) {
            // linear
            envelopeValue = envelopeMax * (phaseTime / attackDurationMs);
        } else {
            // exponential
            double tau = attackDurationMs / attackShapeFactor;
            envelopeValue = envelopeMax * (1.0 - exp(-phaseTime / tau));
        }
    } else if (timeSinceStart < this->attackDurationMs + this->decayDurationMs) {
        // decay phase
        double phaseTime = timeSinceStart - this->attackDurationMs;
        if (attackShapeFactor < 5.0) {
            // linear
            envelopeValue = sustainMax + (envelopeMax - sustainMax) * (1.0 - phaseTime / decayDurationMs);
        } else {
            // exponential
            double tau = decayDurationMs / decayShapeFactor;
            envelopeValue = sustainMax + (envelopeMax - sustainMax) * exp(-phaseTime / tau);
        }
    } else if (timeSinceStart < this->attackDurationMs + this->decayDurationMs + this->sustainDurationMs) {
        // sustain phase
        envelopeValue = this->sustainMax;
    } else if (timeSinceStart < 
            this->attackDurationMs + this->decayDurationMs + this->sustainDurationMs + this->releaseDurationMs) {
        // release phase
        double phaseTime = timeSinceStart - this->attackDurationMs - this->decayDurationMs - this->sustainDurationMs;
        if (releaseShapeFactor < 6.0) {
            // linear
            envelopeValue = this->sustainMax * (1.0 - phaseTime / releaseDurationMs);
        } else {
            // exponential
            double tau = releaseDurationMs / releaseShapeFactor;
            envelopeValue = this->sustainMax * exp(-phaseTime / tau);
        }
    } else {
        envelopeValue = 0.0;
    }
    return envelopeValue;
}

void AdsrEnvelope::setEnvelopeDurationMs(double envelopeDurationMs) {
    double attackDurationMs = this->attackDurationMs;
    double decayDurationMs = this->decayDurationMs;
    double sustainDurationMs = this->sustainDurationMs;
    double releaseDurationMs = this->releaseDurationMs;

    double ratio = envelopeDurationMs / getEnvelopeDurationMs();

    this->attackDurationMs = attackDurationMs * ratio;
    this->decayDurationMs = decayDurationMs * ratio;
    this->sustainDurationMs = sustainDurationMs * ratio;
    this->releaseDurationMs = releaseDurationMs * ratio;
}
