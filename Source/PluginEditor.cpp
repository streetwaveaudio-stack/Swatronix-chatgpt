#include "PluginEditor.h"

namespace
{
    juce::Colour brass() { return juce::Colour(0xffc8a75a); }
    juce::Colour cream() { return juce::Colour(0xffefe4c3); }
    juce::Colour red() { return juce::Colour(0xffa01e24); }

    const juce::StringArray presetNames {
        "1. Velvet Vocal", "2. Smooth Bass", "3. Slow Optical", "4. Broadcast", "5. Warm Room",
        "6. Tight Kick", "7. Acoustic Glow", "8. Vocal Forward", "9. Master Bus Silk", "10. Gentle Levelling"
    };
}

SWATronix2AAudioProcessorEditor::SWATronix2AAudioProcessorEditor(SWATronix2AAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      inputA(p.apvts, "input", input), peakA(p.apvts, "peakReduction", peakReduction),
      gainA(p.apvts, "gain", gain), mixA(p.apvts, "mix", mix), colorA(p.apvts, "color", color),
      hpfA(p.apvts, "hpf", hpf), modeA(p.apvts, "mode", modeBox), themeA(p.apvts, "theme", themeBox),
      makeupA(p.apvts, "autoMakeup", autoMakeup)
{
    setResizable(true, true);
    setResizeLimits(720, 480, 1800, 1200);
    setSize(1120, 680);
    setWantsKeyboardFocus(true);

    displayFace = juce::Typeface::createSystemTypefaceFor("Georgia", juce::Font::plain).get();
    title.setText("SWA TRONIX 2A", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::Font(*displayFace).withHeight(34.0f).boldened());
    title.setColour(juce::Label::textColourId, cream());
    addAndMakeVisible(title);

    company.setText("STREETWAVE AUDIO", juce::dontSendNotification);
    company.setJustificationType(juce::Justification::centred);
    company.setColour(juce::Label::textColourId, brass());
    addAndMakeVisible(company);

    for (auto* knob : { &input, &peakReduction, &gain, &mix, &color, &hpf }) configureKnob(*knob, {});
    input.setName("Input"); peakReduction.setName("Peak Reduction"); gain.setName("Gain"); mix.setName("Mix"); color.setName("Tube Color"); hpf.setName("Detector HPF");

    modeBox.addItem("Compress", 1); modeBox.addItem("Limit", 2);
    modeBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modeBox);

    themeBox.addItem("Photorealistic", 1); themeBox.addItem("Modern", 2); themeBox.addItem("Neon", 3); themeBox.addItem("Motion", 4);
    themeBox.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(themeBox);

    for (int i = 0; i < presetNames.size(); ++i) presetBox.addItem(presetNames[i], i + 1);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.onChange = [this] { applyPreset(presetBox.getSelectedId() - 1); };
    addAndMakeVisible(presetBox);

    autoMakeup.setButtonText("Auto Make-Up");
    autoMakeup.setColour(juce::ToggleButton::textColourId, cream());
    addAndMakeVisible(autoMakeup);

    infoButton.setButtonText("INFO");
    resetButton.setButtonText("RESET");
    infoButton.onClick = [this] { showInfo(); };
    resetButton.onClick = [this] { resetSettings(); };
    addAndMakeVisible(infoButton); addAndMakeVisible(resetButton);

    startTimerHz(30);
}

void SWATronix2AAudioProcessorEditor::configureKnob(juce::Slider& s, const juce::String&)
{
    s.setDoubleClickReturnValue(true, 0.0);
    s.setRange(0.0, 1.0, 0.001);
    s.setColour(juce::Slider::thumbColourId, brass());
    s.setColour(juce::Slider::rotarySliderFillColourId, brass());
    s.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff3d3b33));
    addAndMakeVisible(s);
}

void SWATronix2AAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat();
    const int theme = static_cast<int>(processor.apvts.getRawParameterValue("theme")->load());
    if (theme == 0) drawVintage(g, r);
    else if (theme == 1) drawModern(g, r);
    else if (theme == 2) drawNeon(g, r);
    else drawMotion(g, r);

    juce::Array<juce::Slider*> knobs { &input, &peakReduction, &gain, &mix, &color, &hpf };
    const juce::StringArray captions { "INPUT", "PEAK REDUCTION", "GAIN", "MIX", "TUBE COLOR", "DETECTOR HPF" };
    for (int i = 0; i < knobs.size(); ++i)
    {
        auto b = knobs[i]->getBounds().toFloat();
        g.setColour(theme == 0 ? cream().withAlpha(0.95f) : juce::Colours::white.withAlpha(0.92f));
        g.setFont(juce::Font(12.0f, juce::Font::bold));
        g.drawText(captions[i], b.getX(), b.getBottom() - 24.0f, b.getWidth(), 18.0f, juce::Justification::centred);
    }

    drawMeter(g, { r.getWidth() * 0.42f, r.getHeight() * 0.68f, r.getWidth() * 0.22f, r.getHeight() * 0.20f });
}

void SWATronix2AAudioProcessorEditor::drawVintage(juce::Graphics& g, const juce::Rectangle<float>& r)
{
    g.fillAll(juce::Colour(0xff15130e));
    auto panel = r.reduced(r.getWidth() * 0.035f);
    juce::ColourGradient bg(juce::Colour(0xff4a463a), panel.getTopLeft(), juce::Colour(0xff252319), panel.getBottomRight(), false);
    g.setGradientFill(bg); g.fillRoundedRectangle(panel, 16.0f);
    g.setColour(juce::Colour(0x22000000)); g.fillRoundedRectangle(panel.reduced(8.0f), 12.0f);
    g.setColour(brass()); g.drawRoundedRectangle(panel, 16.0f, 2.0f);
    g.setColour(juce::Colour(0xff181611));
    g.fillRoundedRectangle(panel.withHeight(panel.getHeight() * 0.15f), 14.0f);
    g.setColour(cream());
    g.setFont(juce::Font(*displayFace).withHeight(12.0f).boldened());
    g.drawText("OPTICAL LEVELING AMPLIFIER / 2U VINTAGE SERIES", panel.getX() + 24, panel.getY() + 12, panel.getWidth() - 48, 18, juce::Justification::centred);
    g.setColour(brass());
    g.drawLine(panel.getX() + 20, panel.getY() + 37, panel.getRight() - 20, panel.getY() + 37, 1.5f);
}

void SWATronix2AAudioProcessorEditor::drawModern(juce::Graphics& g, const juce::Rectangle<float>& r)
{
    g.fillAll(juce::Colour(0xff101218));
    auto p = r.reduced(20.0f);
    g.setColour(juce::Colour(0xff1c2029)); g.fillRoundedRectangle(p, 18.0f);
    g.setColour(juce::Colour(0xff343947)); g.drawRoundedRectangle(p, 18.0f, 1.0f);
    g.setColour(juce::Colour(0xfff4f6f8)); g.setFont(juce::Font(30.0f, juce::Font::bold));
    g.drawText("SWA TRONIX 2A", p.getX() + 24, p.getY() + 20, 320, 38, juce::Justification::left);
    g.setColour(juce::Colour(0xff8e96a7)); g.setFont(14.0f); g.drawText("StreetWave Audio • Optical Compressor", p.getX() + 24, p.getY() + 57, 400, 20, juce::Justification::left);
}

void SWATronix2AAudioProcessorEditor::drawNeon(juce::Graphics& g, const juce::Rectangle<float>& r)
{
    g.fillAll(juce::Colour(0xff09050f));
    auto p = r.reduced(14.0f);
    juce::ColourGradient grad(juce::Colour(0xff1a0830), p.getTopLeft(), juce::Colour(0xff09050f), p.getBottomRight(), true);
    g.setGradientFill(grad); g.fillRoundedRectangle(p, 20.0f);
    g.setColour(juce::Colour(0xffd96cff)); g.drawRoundedRectangle(p, 20.0f, 2.0f);
    g.setColour(juce::Colour(0xff6b40ff));
    g.fillEllipse(p.getX() + p.getWidth() - 180.0f, p.getY() + 12.0f, 90.0f, 90.0f);
    g.setColour(juce::Colour(0xfff2d5ff)); g.setFont(30.0f, juce::Font::bold); g.drawText("SWA TRONIX 2A", p.getX() + 24, p.getY() + 22, 400, 40, juce::Justification::left);
}

void SWATronix2AAudioProcessorEditor::drawMotion(juce::Graphics& g, const juce::Rectangle<float>& r)
{
    const float t = animationPhase;
    juce::Colour c1 = juce::Colour::fromHSV(std::fmod(0.62f + 0.15f * std::sin(t), 1.0f), 0.72f, 0.28f, 1.0f);
    juce::Colour c2 = juce::Colour::fromHSV(std::fmod(0.88f + 0.18f * std::cos(t * 0.8f), 1.0f), 0.82f, 0.36f, 1.0f);
    g.fillAll(c1);
    juce::ColourGradient bg(c1, r.getTopLeft(), c2, r.getBottomRight(), true);
    g.setGradientFill(bg); g.fillAll();
    for (int i = 0; i < 8; ++i)
    {
        const float x = r.getX() + r.getWidth() * (0.08f + i * 0.12f) + 25.0f * std::sin(t + i);
        const float y = r.getY() + r.getHeight() * (0.25f + 0.5f * (0.5f + 0.5f * std::sin(t * 0.7f + i)));
        g.setColour(juce::Colour::fromHSV(std::fmod(0.55f + i * 0.06f + t * 0.03f, 1.0f), 0.5f, 0.75f, 0.06f));
        g.fillEllipse(x - 70, y - 70, 140, 140);
    }
    g.setColour(juce::Colours::white.withAlpha(0.9f)); g.setFont(32.0f, juce::Font::bold);
    g.drawText("SWA TRONIX 2A", 30, 30, 420, 40, juce::Justification::left);
}

void SWATronix2AAudioProcessorEditor::drawMeter(juce::Graphics& g, juce::Rectangle<float> area)
{
    const int theme = static_cast<int>(processor.apvts.getRawParameterValue("theme")->load());
    auto outline = theme == 0 ? juce::Colour(0xff231f16) : juce::Colours::black.withAlpha(0.45f);
    auto paper = theme == 0 ? juce::Colour(0xffd7c8a5) : juce::Colour(0xffdce3ee);
    g.setColour(outline); g.fillRoundedRectangle(area.expanded(7.0f), 10.0f);
    g.setColour(paper); g.fillRoundedRectangle(area, 8.0f);
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("GAIN REDUCTION", area.getX(), area.getY() + 5, area.getWidth(), 16, juce::Justification::centred);
    auto inner = area.reduced(16.0f, 25.0f);
    const float v = processor.reductionMeter.load();
    g.setColour(juce::Colours::black.withAlpha(0.2f)); g.fillRect(inner);
    g.setColour(theme == 0 ? red() : juce::Colours::white.withAlpha(0.85f));
    g.fillRect(inner.withWidth(inner.getWidth() * juce::jlimit(0.0f, 1.0f, v)));
}

void SWATronix2AAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(44);
    title.setBounds(area.removeFromTop(42));
    company.setBounds(area.removeFromTop(22));
    area.removeFromTop(10);

    auto top = area.removeFromTop(56);
    presetBox.setBounds(top.removeFromLeft(260));
    themeBox.setBounds(top.removeFromLeft(190));
    modeBox.setBounds(top.removeFromLeft(140));
    infoButton.setBounds(top.removeFromRight(70));
    resetButton.setBounds(top.removeFromRight(75));
    autoMakeup.setBounds(top.removeFromRight(140));
    area.removeFromTop(24);

    auto controls = area.removeFromTop(area.getHeight() * 0.52f);
    const int w = controls.getWidth() / 6;
    juce::Array<juce::Slider*> knobs { &input, &peakReduction, &gain, &mix, &color, &hpf };
    for (auto* k : knobs) k->setBounds(controls.removeFromLeft(w).reduced(8));
}

void SWATronix2AAudioProcessorEditor::timerCallback()
{
    animationPhase += 0.025f;
    if (static_cast<int>(processor.apvts.getRawParameterValue("theme")->load()) == 3)
        repaint();
    else
        repaint(getLocalBounds().removeFromTop(120));
}

void SWATronix2AAudioProcessorEditor::applyPreset(int index)
{
    index = juce::jlimit(0, 9, index);
    static const float values[10][6] = {
        { 0.0f, 42.0f, 18.0f, 100.0f, 18.0f, 35.0f },
        { 0.0f, 48.0f, 14.0f, 100.0f, 24.0f, 30.0f },
        { -1.0f, 34.0f, 20.0f, 100.0f, 15.0f, 40.0f },
        { -2.0f, 55.0f, 16.0f, 100.0f, 20.0f, 55.0f },
        { 0.0f, 30.0f, 17.0f, 100.0f, 28.0f, 32.0f },
        { 1.0f, 62.0f, 12.0f, 85.0f, 14.0f, 65.0f },
        { -1.0f, 26.0f, 19.0f, 100.0f, 22.0f, 28.0f },
        { 0.0f, 46.0f, 21.0f, 100.0f, 17.0f, 36.0f },
        { -3.0f, 20.0f, 11.0f, 70.0f, 8.0f, 20.0f },
        { -1.0f, 18.0f, 16.0f, 100.0f, 10.0f, 25.0f }
    };
    const char* ids[] = { "input", "peakReduction", "gain", "mix", "color", "hpf" };
    for (int i = 0; i < 6; ++i)
        if (auto* param = processor.apvts.getParameter(ids[i]))
        {
            const auto norm = param->convertTo0to1(values[index][i]);
            param->beginChangeGesture(); param->setValueNotifyingHost(norm); param->endChangeGesture();
        }
}

void SWATronix2AAudioProcessorEditor::showInfo()
{
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
        "SWA Tronix 2A",
        "StreetWave Audio\n\nAn original optical-compressor inspired plug-in with a vintage hardware-inspired interface.\n\nVersion 1.0.0\nVST3 / Standalone\n\nCredits: StreetWave Audio", "OK");
}

void SWATronix2AAudioProcessorEditor::resetSettings()
{
    presetBox.setSelectedId(1, juce::dontSendNotification);
    applyPreset(0);
}

bool SWATronix2AAudioProcessorEditor::keyPressed(const juce::KeyPress& k)
{
    if (k.getKeyCode() == 'R') { resetSettings(); return true; }
    return false;
}
