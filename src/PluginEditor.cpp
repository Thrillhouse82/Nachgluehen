#include "PluginEditor.h"

NachgluehenLookAndFeel::NachgluehenLookAndFeel()
{
    setColour(juce::Slider::thumbColourId, juce::Colour(0xfff3b562));
    setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe58b5d));
    setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0x55ffffff));
    setColour(juce::ToggleButton::textColourId, juce::Colour(0xfff4e9dc));
}

void NachgluehenLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(8.0f);
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    g.setColour(juce::Colour(0x223b243b));
    g.fillEllipse(bounds);
    g.setColour(juce::Colour(0x553d5360));
    g.drawEllipse(bounds, 1.5f);
    juce::Path arc;
    arc.addCentredArc(centre.x, centre.y, radius - 3.0f, radius - 3.0f, 0.0f,
                      startAngle, startAngle + sliderPos * (endAngle - startAngle), true);
    g.setColour(findColour(juce::Slider::rotarySliderFillColourId));
    g.strokePath(arc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    const auto angle = startAngle + sliderPos * (endAngle - startAngle);
    g.setColour(juce::Colour(0xfff4e9dc));
    g.fillEllipse(juce::Rectangle<float>(centre.x - 4.0f, centre.y - 4.0f, 8.0f, 8.0f)
                      .withCentre(centre + juce::Point<float>(std::cos(angle), std::sin(angle)) * (radius - 8.0f)));
    juce::ignoreUnused(slider);
}

void NachgluehenLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool hovered, bool down)
{
    const auto area = button.getLocalBounds().toFloat().reduced(2.0f);
    const auto active = button.getToggleState();
    g.setColour(active ? juce::Colour(0xffe58b5d) : juce::Colour(0x443d5360));
    g.fillRoundedRectangle(area, 12.0f);
    g.setColour(active ? juce::Colour(0xffffd39a) : juce::Colour(0x99f4e9dc));
    g.drawRoundedRectangle(area, 12.0f, 1.5f);
    g.setColour(juce::Colour(0xfff4e9dc));
    g.setFont(juce::FontOptions(14.0f).withStyle("bold"));
    g.drawFittedText("FREEZE", button.getLocalBounds(), juce::Justification::centred, 1);
    juce::ignoreUnused(hovered, down);
}

NachgluehenAudioProcessorEditor::NachgluehenAudioProcessorEditor(NachgluehenAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(520, 360);
    setResizable(false, false);
    title.setText("NACHGLUEHEN", juce::dontSendNotification);
    title.setFont(juce::FontOptions(24.0f).withStyle("bold"));
    title.setColour(juce::Label::textColourId, juce::Colour(0xfff4e9dc));
    title.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title);
    freezeButton.setClickingTogglesState(true);
    addAndMakeVisible(freezeButton);
    for (auto* slider : { &driftSlider, &dryWetSlider, &outputGainSlider })
    {
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        slider->setTextValueSuffix({});
        addAndMakeVisible(*slider);
    }
    driftLabel.setText("DRIFT", juce::dontSendNotification);
    dryWetLabel.setText("DRY / WET", juce::dontSendNotification);
    outputGainLabel.setText("OUTPUT", juce::dontSendNotification);
    for (auto* label : { &driftLabel, &dryWetLabel, &outputGainLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        label->setColour(juce::Label::textColourId, juce::Colour(0xffd9c5b5));
        addAndMakeVisible(*label);
    }
    outputGainSlider.setTextValueSuffix(" dB");
    clipLabel.setText("CLIP", juce::dontSendNotification);
    clipLabel.setJustificationType(juce::Justification::centred);
    clipLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff4d4d));
    clipLabel.setFont(juce::FontOptions(12.0f).withStyle("bold"));
    clipLabel.setVisible(false);
    addAndMakeVisible(clipLabel);
    freezeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, nachgluehen::parameterIds::freeze, freezeButton);
    driftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, nachgluehen::parameterIds::drift, driftSlider);
    dryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, nachgluehen::parameterIds::dryWet, dryWetSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, nachgluehen::parameterIds::outputGain, outputGainSlider);
    startTimerHz(30);
}

NachgluehenAudioProcessorEditor::~NachgluehenAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void NachgluehenAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds().toFloat();
    juce::ColourGradient background(juce::Colour(0xff11141d), area.getTopLeft(), juce::Colour(0xff2b1d2d), area.getBottomRight(), false);
    g.setGradientFill(background);
    g.fillAll();
    g.setColour(juce::Colour(0x332f6670));
    g.fillEllipse(area.reduced(area.getWidth() * 0.18f, area.getHeight() * 0.15f));
    g.setColour(juce::Colour(0x187ba58e));
    g.fillEllipse(area.withX(area.getWidth() * 0.45f).withY(area.getHeight() * 0.2f).reduced(10.0f));
}

void NachgluehenAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);
    title.setBounds(area.removeFromTop(42));
    freezeButton.setBounds(area.removeFromTop(38).withSizeKeepingCentre(150, 34));
    area.removeFromTop(18);
    const auto controlWidth = area.getWidth() / 3;
    auto left = area.removeFromLeft(controlWidth).reduced(6);
    auto centre = area.removeFromLeft(controlWidth).reduced(6);
    auto right = area.reduced(6);
    driftSlider.setBounds(left.removeFromTop(left.getHeight() - 30));
    driftLabel.setBounds(left);
    dryWetSlider.setBounds(centre.removeFromTop(centre.getHeight() - 30));
    dryWetLabel.setBounds(centre);
    outputGainSlider.setBounds(right.removeFromTop(right.getHeight() - 48));
    outputGainLabel.setBounds(right.removeFromTop(24));
    clipLabel.setBounds(right.removeFromTop(20));
}

void NachgluehenAudioProcessorEditor::timerCallback()
{
    clipLabel.setVisible(processor.isClipHoldActive());
    repaint();
}
