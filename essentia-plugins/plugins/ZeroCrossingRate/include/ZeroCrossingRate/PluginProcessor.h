#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <essentia/algorithmfactory.h>
#include <essentia/essentiamath.h>

// Forward declaration
class AudioPluginAudioProcessorEditor;

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool                        hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool   acceptsMidi() const override;
    bool   producesMidi() const override;
    bool   isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int                getNumPrograms() override;
    int                getCurrentProgram() override;
    void               setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void               changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    float getZeroCrossingRateValue() const { return zeroCrossingValue; }

    double getEstimatedFrequency() const { return zeroCrossingValue * getSampleRate() / 2.0; }

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    void setEditor(juce::AudioProcessorEditor* editor);
    void removeEditor();

private:
    juce::AudioProcessorValueTreeState                  apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    void                                                updateParameters();

    essentia::standard::Algorithm* zeroCrossingAlg{nullptr};
    std::vector<essentia::Real>    inBuffer;
    essentia::Real                 zeroCrossingThreshold{0.f};
    essentia::Real                 zeroCrossingValue{0.f};

    juce::AudioProcessorEditor* currentEditor{nullptr};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
