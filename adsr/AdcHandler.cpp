#include "AdcHandler.h"

#include <Arduino.h>
#include <driver/adc.h>

void AdcHandler::setup() {
    gpio_num_t adc_gpio = GPIO_NUM_37;
    gpio_set_direction(adc_gpio, GPIO_MODE_INPUT);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_11);  // ADC_CHANNEL_1 = pin 37
    Serial.println("ADC set up.");
}

void AdcHandler::tick() {
    if (micros() - m_lastSampleTime < 100) {
        return; 
    }
    m_lastSampleTime = micros();

    m_lastSampleTime = micros();  // Update the time for the next sample

    switch (m_state) {
        case SAMPLE_TAKEN:
            takeSample();
            m_sampleIndex++;
            if (m_sampleIndex >= NUM_SAMPLES) {
                m_state = PROCESS_SAMPLES;
            }
            break;

        case PROCESS_SAMPLES:
            processSamples();
            m_sampleIndex = 0;
            m_state = SAMPLE_TAKEN;
            break;
    }
}

void AdcHandler::takeSample() {
    int raw = adc1_get_raw(ADC1_CHANNEL_1);
    // Vout = Dout * Vmax / Dmax 
    m_values[m_sampleIndex] = raw * 3.3 / 4095;
}

void AdcHandler::processSamples() {
    double average = 0.0;
    double max = 0.0;
    double min = 1000.0;
    for (int i = 0; i < NUM_SAMPLES; i++) {
        average += m_values[i];
        if (m_values[i] > max) {
            max = m_values[i];
        }
        if (m_values[i] < min) {
            min = m_values[i];
        }
    }
    average /= NUM_SAMPLES;
    double pkpk = max - min;

    if (pkpk * 1000 > 1050 && !m_isNoteOn) {
        m_isNoteOn = true;
        Serial.print("Note On: pk-pk: ");
        Serial.print(pkpk * 1000, 0);
        Serial.println("mv");

        onAdcChanged();
    } else if (pkpk * 1000 < 900 && m_isNoteOn) {
        m_isNoteOn = false;
        Serial.print("Note Off: pk-pk: ");
        Serial.print(pkpk * 1000, 0);
        Serial.println("mv");
        onAdcChanged();
    }

    /* 
    unsigned long currentTime = millis();
    if (currentTime - lastSampleTime < 1000) {
      return;
    }
    lastSampleTime = currentTime;

    if ((pkpk * 1000) < 420) { return; } // not sure why so noisy.

    Serial.print("ADC voltage - min: ");
    Serial.print(min * 1000, 0);
    Serial.print("mv, max: ");
    Serial.print(max * 1000, 0);
    Serial.print("mv, pk-pk: ");
    Serial.print(pkpk * 1000, 0);
    Serial.print("mv, avg: ");
    Serial.print(average * 1000, 0);
    Serial.print("mv [");

    for (int i = 0; i < NUM_SAMPLES; i++) {
        Serial.print(m_values[i], 2);
        Serial.print(", ");
    }
    Serial.println("]");
    */
}