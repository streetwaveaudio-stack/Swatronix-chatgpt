#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr float pi = 3.14159265358979323846f;

    juce::AudioParameterFloatAttributes centred(float midpoint, const juce::String& suffix = {})
    {
        juce::AudioParameterFloatAttributes a;
        a = a.withLabel(suffix).withSliderRange(juce::NormalisableRange<float>(-1.0f, 1.0f, 0.001f), midpoint);
        return a;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SWATronix2AAudioProcessor::createParameterLayout()
{
    using APVTS = juce::AudioProcessorValueTreeState;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterFloat>("input", "Input", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.01f), 0.0f, "dB"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("peakReduction", "Peak Reduction", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 38.0f, "%"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", juce::NormalisableRange<float>(0.0f, 30.0f, 0.01f), 18.0f, "dB"));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("mode", "Mode", juce::StringArray { "Compress", "Limit" }, 0));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 100.0f, "%"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("color", "Tube Color", juce::NormalisableRange<float>(0.0f, 100.0f, 0.01f), 22.0f, "%"));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("hpf", "Detector HPF", juce::NormalisableRange<float>(0.0f, 300.0f, 0.1f), 35.0f, "Hz"));
    p.push_back(std::make_unique<juce::AudioParameterBool>("autoMakeup", "Auto Makeup", true));
    p.push_back(std::make_unique<juce::AudioParameterChoice>("theme", "Theme", juce::StringArray { "Photorealistic", "Modern", "Neon", "Motion" }, 0));
    return { p.begin(), p.end() };
}

SWATronix2AAudioProcessor::SWATronix2AAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                   .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "SWA_TRONIX_2A", createParameterLayout())
{
}

void SWATronix2AAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    inputSmoothed.reset(sampleRate, 0.015);
    outputSmoothed.reset(sampleRate, 0.030);
    reductionSmoothed.reset(sampleRate, 0.010);
    inputSmoothed.setCurrentAndTargetValue(1.0f);
    outputSmoothed.setCurrentAndTargetValue(1.0f);
    reductionSmoothed.setCurrentAndTargetValue(0.0f);
    meterRelease = std::exp(-1.0f / (0.35f * static_cast<float>(sampleRate)));
    detector = {};
    juce::ignoreUnused(samplesPerBlock);
}

void SWATronix2AAudioProcessor::releaseResources() {}

bool SWATronix2AAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    return (mainIn == juce::AudioChannelSet::mono() || mainIn == juce::AudioChannelSet::stereo()) && mainIn == mainOut;
}

void SWATronix2AAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();
    if (channels == 0) return;

    const float inputDb = apvts.getRawParameterValue("input")->load();
    const float peakReduction = apvts.getRawParameterValue("peakReduction")->load();
    const float gainDb = apvts.getRawParameterValue("gain")->load();
    const int mode = static_cast<int>(apvts.getRawParameterValue("mode")->load());
    const float mix = apvts.getRawParameterValue("mix")->load() * 0.01f;
    const float color = apvts.getRawParameterValue("color")->load() * 0.01f;
    const float hpHz = apvts.getRawParameterValue("hpf")->load();
    const bool autoMakeup = apvts.getRawParameterValue("autoMakeup")->load() > 0.5f;

    inputSmoothed.setTargetValue(juce::Decibels::decibelsToGain(inputDb));
    float makeupDb = autoMakeup ? juce::jmap(peakReduction, 0.0f, 100.0f, 0.0f, 10.0f) : 0.0f;
    outputSmoothed.setTargetValue(juce::Decibels::decibelsToGain(gainDb + makeupDb));

    float inputPeak = 0.0f;
    float outputPeak = 0.0f;
    float maxReduction = 0.0f;

    // Program approximation of the optical/variable-mu style response: a level-dependent detector,
    // slow optical attack/release, soft knee, asymmetric timing, and gentle tube saturation.
    const float attackMs = mode == 0 ? 10.0f : 2.5f;
    const float releaseMs = mode == 0 ? 420.0f : 220.0f;
    const float attackCoeff = std::exp(-1.0f / (attackMs * 0.001f * static_cast<float>(currentSampleRate)));
    const float releaseCoeff = std::exp(-1.0f / (releaseMs * 0.001f * static_cast<float>(currentSampleRate)));

    float dcX = 0.0f;
    float dcY = 0.0f;
    const float hpAlpha = hpHz <= 1.0f ? 1.0f : std::exp(-2.0f * pi * hpHz / static_cast<float>(currentSampleRate));

    for (int s = 0; s < numSamples; ++s)
    {
        const float inGain = inputSmoothed.getNextValue();
        float detectorInput = 0.0f;
        float inAbs = 0.0f;

        for (int ch = 0; ch < channels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            const float original = data[s];
            const float x = original * inGain;
            data[s] = x;
            inAbs = std::max(inAbs, std::abs(x));
            detectorInput += x * x;
        }
        detectorInput = std::sqrt(detectorInput / static_cast<float>(channels) + 1.0e-12f);
        if (hpHz > 1.0f)
        {
            const float y = detectorInput - dcX + hpAlpha * dcY;
            dcX = detectorInput;
            dcY = y;
            detectorInput = std::abs(y);
        }

        detector.rms = detector.rms * 0.995f + detectorInput * 0.005f;
        const float levelDb = juce::Decibels::gainToDecibels(juce::jmax(detector.rms, 1.0e-7f));
        const float drive = juce::jmap(peakReduction, 0.0f, 100.0f, 0.0f, 32.0f);
        const float thresholdDb = mode == 0 ? -18.0f : -12.0f;
        float desiredDb = (levelDb + drive) - thresholdDb;
        desiredDb = juce::jmax(0.0f, desiredDb);
        desiredDb *= mode == 0 ? 0.72f : 1.0f;
        const float maxDb = mode == 0 ? 14.0f : 24.0f;
        desiredDb = juce::jmin(desiredDb, maxDb);

        const float coeff = desiredDb > detector.gainReductionDb ? attackCoeff : releaseCoeff;
        detector.gainReductionDb = coeff * detector.gainReductionDb + (1.0f - coeff) * desiredDb;
        maxReduction = std::max(maxReduction, detector.gainReductionDb);

        float reductionGain = juce::Decibels::decibelsToGain(-detector.gainReductionDb);
        // Tube color: soft asymmetric saturation before/after the optical stage.
        const float sat = 1.0f + color * 3.0f;
        const float preSat = juce::jlimit(-4.0f, 4.0f, (inAbs * sat));
        const float colorComp = 1.0f + 0.08f * color * std::tanh(preSat * 0.7f);
        reductionGain *= colorComp;

        const float outGain = outputSmoothed.getNextValue();
        for (int ch = 0; ch < channels; ++ch)
        {
            auto* data = buffer.getWritePointer(ch);
            float wet = data[s] * reductionGain * outGain;
            wet = std::tanh(wet * (1.0f + 0.8f * color)) / std::tanh(1.0f + 0.8f * color);
            const float dry = data[s] * outGain;
            data[s] = dry + (wet - dry) * mix;
            outputPeak = std::max(outputPeak, std::abs(data[s]));
        }

        inputPeak = std::max(inputPeak, inAbs);
    }

    // Slow VU ballistics.
    const float vuDb = -maxReduction;
    const float vuTarget = juce::jlimit(0.0f, 1.0f, juce::jmap(vuDb, -24.0f, 3.0f, 0.0f, 1.0f));
    meterHold = 0.92f * meterHold + 0.08f * vuTarget;
    reductionSmoothed.setTargetValue(maxReduction);
    reductionSmoothed.skip(numSamples);
    reductionMeter.store(juce::jlimit(0.0f, 1.0f, maxReduction / 24.0f));
    inputMeter.store(juce::jlimit(0.0f, 1.0f, inputPeak));
    outputMeter.store(juce::jlimit(0.0f, 1.0f, outputPeak));
    vuMeter.store(meterHold);
}

void SWATronix2AAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("schema", 1, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void SWATronix2AAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SWATronix2AAudioProcessor();
}
