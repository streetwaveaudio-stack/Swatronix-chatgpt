#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SWATronix2AAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                               private juce::Timer
{
public:
    explicit SWATronix2AAudioProcessorEditor(SWATronix2AAudioProcessor&);
    ~SWATronix2AAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;

private:
    class Knob : public juce::Slider
    {
    public:
        Knob() { setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 20); }
    } input, peakReduction, gain, mix, color, hpf;

    juce::ComboBox modeBox, presetBox, themeBox;
    juce::ToggleButton autoMakeup;
    juce::TextButton infoButton, resetButton;
    juce::Label title, company, modeLabel, presetLabel, themeLabel;
    juce::AudioProcessorValueTreeState::SliderAttachment inputA, peakA, gainA, mixA, colorA, hpfA;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment modeA, themeA;
    juce::AudioProcessorValueTreeState::ButtonAttachment makeupA;

    SWATronix2AAudioProcessor& processor;
    juce::Typeface::Ptr displayFace;
    float animationPhase = 0.0f;
    int currentPreset = 0;

    void timerCallback() override;
    void applyPreset(int index);
    void showInfo();
    void resetSettings();
    void drawVintage(juce::Graphics&, const juce::Rectangle<float>&);
    void drawModern(juce::Graphics&, const juce::Rectangle<float>&);
    void drawNeon(juce::Graphics&, const juce::Rectangle<float>&);
    void drawMotion(juce::Graphics&, const juce::Rectangle<float>&);
    void drawMeter(juce::Graphics&, juce::Rectangle<float> area);
    void configureKnob(juce::Slider&, const juce::String&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SWATronix2AAudioProcessorEditor)
};
