#pragma once

#include <JuceHeader.h>
#include "BinaryData.h"
#include "PluginProcessor.h"

//==============================================================================
// Custom Look and Feel for modern styling
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();
};

//==============================================================================
class EssentiaPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit EssentiaPluginAudioProcessorEditor(EssentiaPluginAudioProcessor&);
    ~EssentiaPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    EssentiaPluginAudioProcessor& processorRef;
    void                       timerCallback() override;

    // UI Components
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label danceabilityValueLabel;
    juce::Label danceabilityUnitLabel;
    juce::Label minTauLabel;
    juce::Label maxTauLabel;
    juce::Label tauMultiplierLabel;
    juce::Label analysisWindowLabel;
    juce::Slider minTauSlider;
    juce::Slider maxTauSlider;
    juce::Slider tauMultiplierSlider;
    juce::Slider analysisWindowSlider;
    std::unique_ptr<SliderAttachment> minTauAttachment;
    std::unique_ptr<SliderAttachment> maxTauAttachment;
    std::unique_ptr<SliderAttachment> tauMultiplierAttachment;
    std::unique_ptr<SliderAttachment> analysisWindowAttachment;

    // Custom styling
    ModernLookAndFeel modernLF;

    // Logos
    juce::Image upfLogo;
    juce::Image essentiaLogo;

    // Additional label for "powered by" text
    juce::Label poweredByLabel;

    // Easter egg method
    void showDeveloperInfo();

    // Hover state for title
    bool titleHovered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EssentiaPluginAudioProcessorEditor)
};
