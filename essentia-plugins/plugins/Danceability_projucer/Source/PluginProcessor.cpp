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

    delete danceability;
    
    danceability = AlgorithmFactory::create("Danceability");
    danceability->configure("sampleRate", sampleRate);

    // connect I/O algorithm
    danceability->input("signal").set(essentiaBuffer);
    danceability->output("danceability").set(danceabilityValue);
    danceability->output("dfa").set(dfaValues);

    constexpr double analysisWindowSeconds = 10.0;
    constexpr double computeIntervalSeconds = 1.0;

    analysisWindowSamples = static_cast<size_t>(juce::jmax(1.0, sampleRate * analysisWindowSeconds));
    computeIntervalSamples = static_cast<size_t>(juce::jmax(1.0, sampleRate * computeIntervalSeconds));
    samplesSinceDanceabilityCompute = computeIntervalSamples;
    danceabilityValue = 0.f;

    essentiaBuffer.clear();
    essentiaBuffer.reserve(analysisWindowSamples + static_cast<size_t>(juce::jmax(1, samplesPerBlock)));
}

void EssentiaPluginAudioProcessor::releaseResources()
{
    delete danceability;
    danceability = nullptr;
    dfaValues.clear();
    essentiaBuffer.clear();
    analysisWindowSamples = 0;
    computeIntervalSamples = 0;
    samplesSinceDanceabilityCompute = 0;
    danceabilityValue = 0.f;
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

    if (danceability == nullptr || analysisWindowSamples == 0 || computeIntervalSamples == 0)
        return;

    const int numSamples = buffer.getNumSamples();
    const int numInputChannels = juce::jmin(buffer.getNumChannels(), getTotalNumInputChannels());
    if (numSamples <= 0 || numInputChannels <= 0)
        return;

    const auto previousSize = essentiaBuffer.size();
    essentiaBuffer.resize(previousSize + static_cast<size_t>(numSamples));

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float monoSample = 0.f;
        for (int channel = 0; channel < numInputChannels; ++channel)
            monoSample += buffer.getSample(channel, sample);

        essentiaBuffer[previousSize + static_cast<size_t>(sample)] = monoSample / static_cast<float>(numInputChannels);
    }

    if (essentiaBuffer.size() > analysisWindowSamples)
    {
        const auto samplesToDrop = essentiaBuffer.size() - analysisWindowSamples;
        essentiaBuffer.erase(essentiaBuffer.begin(), essentiaBuffer.begin() + static_cast<std::ptrdiff_t>(samplesToDrop));
    }

    if (essentiaBuffer.size() < analysisWindowSamples)
        return;

    samplesSinceDanceabilityCompute += static_cast<size_t>(numSamples);
    if (samplesSinceDanceabilityCompute < computeIntervalSamples)
        return;

    samplesSinceDanceabilityCompute = 0;
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
