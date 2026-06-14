/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <algorithmfactory.h>
#include <essentiamath.h>

//==============================================================================
/**
*/
class EssentiaPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    EssentiaPluginAudioProcessor();
    ~EssentiaPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Essentia custom functions
    void initializeEssentiaAlgorithms(int sampleRate, int frameSize);
    void connectBufferToAlgorithms();
    void applyZeroPadding(std::vector<essentia::Real>& buffer, int maxSampleSize);
    
    void loadEssentiaBuffer(std::vector<float> buffer);
    void computeEssentiaAlgorithms();
    void cleanupEssentia();
      
    // --- quick access to the most recent danceability value ----------
    float getDanceability() const { return danceabilityValue; }

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void configureDanceability();

    essentia::standard::Algorithm* danceability = nullptr;
    essentia::Real danceabilityValue = 0.f;
    std::vector<essentia::Real> dfaValues;
    std::vector<essentia::Real> essentiaBuffer;
    size_t analysisWindowSamples = 0;
    size_t computeIntervalSamples = 0;
    size_t samplesSinceDanceabilityCompute = 0;
    double currentSampleRate = 44100.0;
    
    int maxSampleSize {1024};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EssentiaPluginAudioProcessor)
};
