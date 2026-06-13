#pragma once

#include "BinaryData.h"
#include "RMS/PluginProcessor.h"

//==============================================================================
// Custom Look and Feel for modern styling
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();
};

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    void                       timerCallback() override;

    // UI Components
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label rmsLinearValueLabel;
    juce::Label rmsDbValueLabel;
    juce::Label rmsLinearUnitLabel;
    juce::Label rmsDbUnitLabel;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
