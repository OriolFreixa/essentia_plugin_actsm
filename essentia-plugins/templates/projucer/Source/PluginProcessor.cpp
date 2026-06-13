/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VsttemplateAudioProcessor::VsttemplateAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

VsttemplateAudioProcessor::~VsttemplateAudioProcessor()
{
    releaseResources();
}

//==============================================================================
const juce::String VsttemplateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VsttemplateAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VsttemplateAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VsttemplateAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VsttemplateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VsttemplateAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VsttemplateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VsttemplateAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VsttemplateAudioProcessor::getProgramName (int index)
{
    return {};
}

void VsttemplateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VsttemplateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    initializeEssentiaAlgorithms(int(sampleRate), samplesPerBlock);
    DBG("sampleRate: " + juce::String(sampleRate));
    DBG("samplesPerBlock: " + juce::String(samplesPerBlock));
}

void VsttemplateAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    cleanupEssentia();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VsttemplateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void VsttemplateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    // TODO: avoid processing when the DAW is stopped or when there is no audio clip in the timeline
    std::vector<float> audioBuffer = applyZeroPadding(buffer, maxSampleSize);
    loadEssentiaBuffer(audioBuffer);
    
    computeEssentiaAlgorithms();
    
    DBG("rms value: " + juce::String(rmsValue) + " - " + juce::String(amp2db(rmsValue)) + "dB");
    DBG("energy value: " + juce::String(energyValue));
}

//==============================================================================
bool VsttemplateAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VsttemplateAudioProcessor::createEditor()
{
    return new VsttemplateAudioProcessorEditor (*this);
}

//==============================================================================
void VsttemplateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VsttemplateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VsttemplateAudioProcessor();
}

// Essentia custom functionalities

void VsttemplateAudioProcessor::initializeEssentiaAlgorithms(int sampleRate, int frameSize)
{
    // initialize essentia algorithms
    essentia::init();
    AlgorithmFactory& factory = standard::AlgorithmFactory::instance();
    
    // check the parameters for each algo
    rms = factory.create("RMS");
    // to define parameter values use <parameter_id>, <parameter_value>
    energy = factory.create("Energy");

    // connect algorithm I/Os
    rms->input("array").set(essentiaBuffer);
    rms->output("rms").set(rmsValue);
    
    energy->input("array").set(essentiaBuffer);
    energy->output("energy").set(energyValue);
    
};

void VsttemplateAudioProcessor::connectBufferToAlgorithms()
{
    rms->input("frame").set(essentiaBuffer);
    rms->output("frame").set(rmsValue);
    
    energy->input("signal").set(essentiaBuffer);
    energy->output("signal").set(energyValue);
}

std::vector<float> VsttemplateAudioProcessor::applyZeroPadding(juce::AudioBuffer<float> buffer, int maxSampleSize){
    
    float* start = buffer.getWritePointer(0); // get the pointer to the first sample of the first channel
    int size = buffer.getNumSamples();
    if (size < maxSampleSize){
        // get an array of padded zeros
        int padded_zeros_size =  maxSampleSize - size;
        std::vector<float> audio_buffer_vec(start, start + size); // this will copy the data as a vector
        // append array of padded zeros to audio_buffer
        for(int i=0; i < padded_zeros_size; i++){
            audio_buffer_vec.push_back(0.0f);
        }
        return audio_buffer_vec;

    }
    else{
        std::vector<float> audio_buffer_vec(start, start + maxSampleSize);
        return audio_buffer_vec;
    }

}

void VsttemplateAudioProcessor::loadEssentiaBuffer(std::vector<float> buffer)
{
    vector<Real> audio_buffer;
    for(auto sample : buffer){
        audio_buffer.push_back(sample);
    }
    essentiaBuffer = audio_buffer;
};

void VsttemplateAudioProcessor::computeEssentiaAlgorithms()
{
    rms->compute();
    energy->compute();
}

void VsttemplateAudioProcessor::cleanupEssentia() {
    essentiaBuffer.clear();
    essentia::shutdown();
    //delete rms;
    //delete energy;
}

