/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <essentia.h>
#include <stdio.h>
#include <vector>
#include <algorithmfactory.h>
#include <essentiamath.h>

using namespace essentia;
using namespace essentia::standard;
using namespace std;

//==============================================================================
/**
*/
class VsttemplateAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    VsttemplateAudioProcessor();
    ~VsttemplateAudioProcessor() override;

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
    
    std::vector<float> applyZeroPadding(juce::AudioBuffer<float> buffer, int maxSampleSize);
    void loadEssentiaBuffer(std::vector<float> buffer);
    void computeEssentiaAlgorithms();
    void cleanupEssentia();
    
    float getRMS() const { return (float)amp2db(rmsValue); }
    float getEnergy() const { return (float)energyValue; }
    
private:
    //==============================================================================
    
    Algorithm* rms;
    Algorithm* energy;
    
    vector<Real> essentiaBuffer;
    Real rmsValue, energyValue;
    
    int maxSampleSize = { 1024 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VsttemplateAudioProcessor)
};
