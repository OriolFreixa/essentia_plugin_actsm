#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithmfactory.h>
#include <essentiamath.h>

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

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    juce::AudioProcessorValueTreeState                  apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters() const;
    void                                                updateParameters();
    void                                                reloadAlgorithm();
    bool createAudio2MidiAlgorithm(int samplesPerBlock, double sampleRate, bool useCurrentParameters = false);

    essentia::standard::Algorithm* audio2midi{nullptr};
    // std::vector<essentia::Real>    inBuffer;
    // essentia::Real                 energyValue{0.f};

    // audio2midi inputs and outputs
    std::vector<essentia::Real> inputFrame;
    essentia::Real              rmsValue, pitch, rms;
    std::vector<std::string>    messageType;
    std::vector<essentia::Real> midiNoteNumber;
    std::vector<essentia::Real> timeCompensation;

    // timestamp
    juce::AudioPlayHead::CurrentPositionInfo lastPosInfo;

    double currentTime, msPerSample, mSampleRate;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
