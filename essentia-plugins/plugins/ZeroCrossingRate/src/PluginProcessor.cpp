#include "ZeroCrossingRate/PluginProcessor.h"
#include "ZeroCrossingRate/PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         )
    , apvts(*this, nullptr, "Parameters", createParameters())
{}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    releaseResources();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters() const
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("zeroCrossingThreshold",
                                                           "Zero Crossing Threshold",
                                                           0.0f,
                                                           1.0f,
                                                           0.3f));

    return layout;
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
              // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(sampleRate, samplesPerBlock);
    essentia::init();
    auto& factory = essentia::standard::AlgorithmFactory::instance();

    // create algorithm once
    zeroCrossingAlg = factory.create("ZeroCrossingRate", "threshold", zeroCrossingThreshold);

    // allocate vector once to max block size
    inBuffer.resize(static_cast<std::size_t>(samplesPerBlock));

    // connect ports once
    zeroCrossingAlg->input("signal").set(inBuffer);
    zeroCrossingAlg->output("zeroCrossingRate").set(zeroCrossingValue);

    // prime to avoid heap allocs in the audio thread
    std::fill(inBuffer.begin(), inBuffer.end(), 0.f);
    zeroCrossingAlg->compute();
}

void AudioPluginAudioProcessor::releaseResources()
{
    delete zeroCrossingAlg;
    zeroCrossingAlg = nullptr;
    inBuffer.clear();
    essentia::shutdown();
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::updateParameters()
{
    // Get the current parameter value
    auto newThreshold = static_cast<essentia::Real>(apvts.getRawParameterValue("zeroCrossingThreshold")->load());

    // Use epsilon comparison for floating point values
    constexpr float epsilon = 0.000001f;
    if (std::abs(newThreshold - zeroCrossingThreshold) < epsilon)
        return; // no significant change

    zeroCrossingThreshold = newThreshold;

    if (zeroCrossingAlg != nullptr)
    {
        zeroCrossingAlg->configure("threshold", zeroCrossingThreshold);
        DBG("ZeroCrossingRate threshold updated to: " << zeroCrossingThreshold);
    }
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    updateParameters();

    const int n = buffer.getNumSamples();

    // copy first channel to Essentia vector
    const float* read = buffer.getReadPointer(0);
    inBuffer.assign(read, read + n);

    // run algorithm
    zeroCrossingAlg->compute();

    // Pass audio data to editor for waveform visualization
    if (currentEditor != nullptr)
    {
        auto* pluginEditor = dynamic_cast<AudioPluginAudioProcessorEditor*>(currentEditor);
        if (pluginEditor != nullptr)
        {
            pluginEditor->pushAudioData(read, n);
        }
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Store the parameter values using the AudioProcessorValueTreeState
    auto                              state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore the parameter values from the stored state
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        // Make sure to update our local threshold value after state is restored
        updateParameters();
    }
}

//==============================================================================
// This creates new instances of the plugin..
void AudioPluginAudioProcessor::setEditor(juce::AudioProcessorEditor* editor)
{
    currentEditor = editor;
}

void AudioPluginAudioProcessor::removeEditor()
{
    currentEditor = nullptr;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
