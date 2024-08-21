#ifndef ADC_HANDLER_H
#define ADC_HANDLER_H

class AdcHandler {
   public:
    // the signal is centered on 1100mv
    static constexpr double TRIGGER_ON_PKPK_MV = 275.0;
    static constexpr double TRIGGER_OFF_PKPK_MV = 190.0;

    void setup();
    void tick();
    using OnAdcChanged = void(*)();
    void registerOnAdcChange(OnAdcChanged callback) { onAdcChanged = callback; }
    bool isNoteOn() { return m_isNoteOn; };

   private:
    enum State {
        SAMPLE_TAKEN,
        PROCESS_SAMPLES
    };

    State m_state = SAMPLE_TAKEN;
    static constexpr int NUM_SAMPLES = 200;
    double m_values[NUM_SAMPLES];
    int m_sampleIndex = 0;
    bool m_isNoteOn = false;
    unsigned long m_lastSampleTime = 0;
    OnAdcChanged onAdcChanged;
    long lastSampleTime = 0;

    void takeSample();
    void processSamples();
};

#endif