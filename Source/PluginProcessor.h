#pragma once
#include <JuceHeader.h>

class SWATronix2AAudioProcessor final : public juce::AudioProcessor
{
public:
    SWATronix2AAudioProcessor();
    ~SWATronix2AAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> inputMeter { 0.0f };
    std::atomic<float> outputMeter { 0.0f };
    std::atomic<float> reductionMeter { 0.0f };
    std::atomic<float> vuMeter { 0.0f };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    struct Detector
    {
        float rms = 0.0f;
        float peak = 0.0f;
        float gainReductionDb = 0.0f;
        float attackState = 0.0f;
        float releaseState = 0.0f;
    } detector;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> inputSmoothed { 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> outputSmoothed { 1.0f };
    juce::SmoothedValue<float> reductionSmoothed { 0.0f };

    double currentSampleRate = 44100.0;
    float meterHold = 0.0f;
    float meterRelease = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SWATronix2AAudioProcessor)
};

