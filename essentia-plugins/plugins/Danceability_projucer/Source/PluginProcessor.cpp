/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using essentia::standard::AlgorithmFactory;
using essentia::Real;


//==============================================================================
// clang-format off

EssentiaPluginAudioProcessor::EssentiaPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
}
// clang-format on

EssentiaPluginAudioProcessor::~EssentiaPluginAudioProcessor() {}

//==============================================================================
const juce::String EssentiaPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EssentiaPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EssentiaPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EssentiaPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EssentiaPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EssentiaPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EssentiaPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EssentiaPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String EssentiaPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void EssentiaPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void EssentiaPluginAudioProcessor::applyZeroPadding(std::vector<Real>& buffer, int maxSampleSize)
{
    const int currentSize = static_cast<int>(buffer.size());
    
    if (currentSize < maxSampleSize)
    {
        // Reserve and fill with zeros - much faster than push_back loop
        buffer.reserve(maxSampleSize);
        buffer.resize(maxSampleSize, 0.0f);  // Single operation!
    }
    else if (currentSize > maxSampleSize)
    {
        // Trim to desired size
        buffer.resize(maxSampleSize);
    }
    // If equal, do nothing
}

//==============================================================================
void EssentiaPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    
    essentia::init();
    
    danceability = AlgorithmFactory::create("Danceability");
    danceability->configure("sampleRate", sampleRate);

    // connect I/O algorithm
    danceability->input("signal").set(essentiaBuffer);
    danceability->output("danceability").set(danceabilityValue);
    danceability->output("dfa").set(dfaValues);
  
    // prime once with a dummy frame to avoid allocations in the audio thread
    const auto analysisSamples = static_cast<size_t>(juce::jmax(1.0, sampleRate));
    essentiaBuffer.assign(analysisSamples, 0.f);
    danceability->compute();
}

void EssentiaPluginAudioProcessor::releaseResources()
{
    delete danceability;
    danceability = nullptr;
    dfaValues.clear();
    essentiaBuffer.clear();
    essentia::shutdown();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EssentiaPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EssentiaPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    // process single-channel for now
    const float* in = buffer.getReadPointer(0);
    essentiaBuffer.assign(in, in + buffer.getNumSamples());
    
    const auto minFrameSamples = static_cast<size_t>(juce::jmax(1.0, getSampleRate() * 0.01));
    if (essentiaBuffer.size() < minFrameSamples)
        essentiaBuffer.resize(minFrameSamples, 0.f);

    danceability->compute();
}

//==============================================================================
bool EssentiaPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EssentiaPluginAudioProcessor::createEditor()
{
    return new EssentiaPluginAudioProcessorEditor (*this);
}

//==============================================================================
void EssentiaPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void EssentiaPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EssentiaPluginAudioProcessor();
}
