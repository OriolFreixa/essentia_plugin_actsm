#pragma once

#include "BinaryData.h"
#include "ZeroCrossingRate/PluginProcessor.h"

//==============================================================================
// Custom waveform component for visual feedback
class WaveformComponent : public juce::Component, private juce::Timer
{
public:
    WaveformComponent();
    ~WaveformComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void setThreshold(float threshold);
    void pushAudioData(const float* audioData, int numSamples);

private:
    void timerCallback() override;

    std::vector<float>   audioBuffer;
    float                currentThreshold = 0.35f;
    static constexpr int maxBufferSize    = 1024; // Samples to display
    std::atomic<bool>    bufferNeedsUpdate{false};
};

//==============================================================================
// Custom Look and Feel for modern styling
class ModernLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModernLookAndFeel();

    void drawLinearSlider(juce::Graphics&                 g,
                          int                             x,
                          int                             y,
                          int                             width,
                          int                             height,
                          float                           sliderPos,
                          float                           minSliderPos,
                          float                           maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider&                   slider) override;

    void drawLinearSliderThumb(juce::Graphics&                 g,
                               int                             x,
                               int                             y,
                               int                             width,
                               int                             height,
                               float                           sliderPos,
                               float                           minSliderPos,
                               float                           maxSliderPos,
                               const juce::Slider::SliderStyle style,
                               juce::Slider&                   slider) override;
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
    void pushAudioData(const float* audioData, int numSamples);
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseMove(const juce::MouseEvent& event) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    void                       timerCallback() override;

    // UI Components
    juce::Slider thresholdSlider;
    juce::Label  thresholdLabel;
    juce::Label  thresholdValueLabel;
    juce::Label  titleLabel;
    juce::Label  subtitleLabel;
    juce::Label  zcrValueLabel;
    juce::Label  zcrUnitLabel;

    WaveformComponent waveformComponent;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;

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
