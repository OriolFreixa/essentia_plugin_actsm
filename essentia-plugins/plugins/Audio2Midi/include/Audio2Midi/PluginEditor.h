#pragma once

#include "Audio2Midi/PluginProcessor.h"
#include "BinaryData.h"

//==============================================================================
// Custom MIDI visualization component
class MidiVisualizationComponent : public juce::Component, private juce::Timer
{
public:
    MidiVisualizationComponent();
    ~MidiVisualizationComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void triggerMidiNote(int noteNumber, float velocity);

private:
    void timerCallback() override;

    struct MidiNote
    {
        int          noteNumber;
        float        velocity;
        float        position;
        float        width;
        juce::Colour colour;
    };

    std::vector<MidiNote> activeNotes;
    std::atomic<bool>     notesNeedUpdate{false};
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

    void drawToggleButton(juce::Graphics&     g,
                          juce::ToggleButton& button,
                          bool                shouldDrawButtonAsHighlighted,
                          bool                shouldDrawButtonAsDown) override;
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

    // UI Components - Frequency Range
    juce::Slider minFrequencySlider;
    juce::Label  minFrequencyLabel;
    juce::Label  minFrequencyValueLabel;

    juce::Slider maxFrequencySlider;
    juce::Label  maxFrequencyLabel;
    juce::Label  maxFrequencyValueLabel;

    // UI Components - Tuning & Detection
    juce::Slider tuningFrequencySlider;
    juce::Label  tuningFrequencyLabel;
    juce::Label  tuningFrequencyValueLabel;

    juce::Slider pitchConfidenceSlider;
    juce::Label  pitchConfidenceLabel;
    juce::Label  pitchConfidenceValueLabel;

    juce::Slider loudnessThresholdSlider;
    juce::Label  loudnessThresholdLabel;
    juce::Label  loudnessThresholdValueLabel;

    // UI Components - MIDI Processing
    juce::Slider transpositionSlider;
    juce::Label  transpositionLabel;
    juce::Label  transpositionValueLabel;

    juce::ToggleButton timeCompensationToggle;
    juce::Label        timeCompensationLabel;

    // UI Components - System
    juce::TextButton reloadButton;

    // UI Components - General
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label statusLabel;

    // Group labels
    juce::Label frequencyGroupLabel;
    juce::Label tuningGroupLabel;
    juce::Label midiGroupLabel;
    juce::Label systemGroupLabel;

    // MIDI visualization
    MidiVisualizationComponent midiVisualization;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> minFrequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> maxFrequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tuningFrequencyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pitchConfidenceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loudnessThresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> transpositionAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> timeCompensationAttachment;

    // Custom styling
    ModernLookAndFeel modernLF;

    // Logos (if you want to add them later)
    juce::Image upfLogo;
    juce::Image essentiaLogo;
    juce::Label poweredByLabel;

    // Reload state tracking
    bool isReloading = false;

    // Helper methods
    void setupSlider(juce::Slider&       slider,
                     juce::Label&        label,
                     juce::Label&        valueLabel,
                     const juce::String& labelText,
                     const juce::String& parameterID);
    void updateValueLabel(juce::Label& valueLabel, float value, const juce::String& unit, int decimals = 0);

    // Easter egg method
    void showDeveloperInfo();

    // Hover state for title
    bool titleHovered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};