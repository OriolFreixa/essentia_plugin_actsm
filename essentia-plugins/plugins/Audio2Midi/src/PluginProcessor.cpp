#include "Audio2Midi/PluginProcessor.h"
#include "Audio2Midi/PluginEditor.h"

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

// clang-format off
/*
void declareParameters() {
        declareParameter("sampleRate", "sample rate of incoming audio frames", "[8000,inf)", 44100);
        declareParameter("hopSize", "equivalent to I/O buffer size", "[1,inf)", 32);
        declareParameter("minFrequency", "minimum frequency to detect in Hz", "[10,20000]", 60.0);
        declareParameter("maxFrequency", "maximum frequency to detect in Hz", "[10,20000]", 2300.0);
        declareParameter("tuningFrequency", "tuning frequency for semitone index calculation, corresponding to A3 [Hz]", "{432,440}", 440);
        declareParameter("pitchConfidenceThreshold", "level of pitch confidence above which note ON/OFF start to be considered", "[0,1]", 0.25);
        declareParameter("loudnessThreshold", "loudness level above/below which note ON/OFF start to be considered, in decibels", "[-inf,0]", -51.0);
        declareParameter("transpositionAmount", "Apply transposition (in semitones) to the detected MIDI notes.", "(-69,50)", 0);
        declareParameter("minOccurrenceRate", "rate of predominant pitch occurrence in MidiPool buffer to consider note ON event", "[0,1]", 0.5);
        declareParameter("midiBufferDuration", "duration in seconds of buffer used for voting in MidiPool algorithm", "[0.005,0.5]", 0.05); // 15ms
        declareParameter("minNoteChangePeriod", "minimum time to wait until a note change is detected (testing only)", "(0,1]", 0.030);
        declareParameter("minOnsetCheckPeriod", "minimum time to wait until an onset is detected (testing only)", "(0,1]", 0.075);
        declareParameter("minOffsetCheckPeriod", "minimum time to wait until an offset is detected (testing only)", "(0,1]", 0.2);
        declareParameter("applyTimeCompensation", "whether to apply time compensation correction to MIDI note detection", "{true,false}", true);
      }
*/
// clang-format on

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters() const
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("minFrequency",
                                                           "Minimum Frequency",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.1f),
                                                           60.0f,
                                                           "Hz"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("maxFrequency",
                                                           "Maximum Frequency",
                                                           juce::NormalisableRange<float>(10.0f, 20000.0f, 1.0f, 0.1f),
                                                           2300.0f,
                                                           "Hz"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("tuningFrequency",
                                                           "Tuning Frequency",
                                                           juce::NormalisableRange<float>(432.0f, 440.0f, 1.0f, 0.1f),
                                                           440.0f,
                                                           "Hz"));
    layout.add(std::make_unique<juce::AudioParameterFloat>("pitchConfidenceThreshold",
                                                           "Pitch Confidence Threshold",
                                                           0.0f,
                                                           1.0f,
                                                           0.25f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("loudnessThreshold",
                                                           "Loudness Threshold",
                                                           juce::NormalisableRange<float>(-80, 0.0f, 0.1f, 0.1f),
                                                           -51.0f,
                                                           "dB"));
    layout.add(std::make_unique<juce::AudioParameterInt>("transpositionAmount",
                                                         "Transposition Amount",
                                                         -69,
                                                         50,
                                                         0,
                                                         "Semitones"));
    layout.add(std::make_unique<juce::AudioParameterBool>("applyTimeCompensation",
                                                          "Apply Time Compensation",
                                                          false,
                                                          "Boolean"));
    layout.add(std::make_unique<juce::AudioParameterBool>("reloadAlgorithm", "Reload Algorithm", false, "Button"));
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

    // Validate parameters
    if (sampleRate <= 0 || samplesPerBlock <= 0)
    {
        DBG("Invalid audio parameters: sampleRate=" + juce::String(sampleRate) +
            ", samplesPerBlock=" + juce::String(samplesPerBlock));
        return;
    }

    // Resize the essentia buffer to avoid allocations during processing
    inputFrame.clear();
    inputFrame.resize(static_cast<std::size_t>(samplesPerBlock));

    // Create the Audio2Midi algorithm with default parameters
    if (!createAudio2MidiAlgorithm(samplesPerBlock, sampleRate, false))
    {
        DBG("Failed to initialize Audio2Midi algorithm in prepareToPlay");
        return;
    }

    /*DBG("sampleRate: " + juce::String(sampleRate));
    DBG("samplesPerBlock: " + juce::String(samplesPerBlock));*/

    msPerSample = 1000 / sampleRate;
    mSampleRate = sampleRate;

    currentTime = 0;
}

void AudioPluginAudioProcessor::releaseResources()
{
    if (audio2midi != nullptr)
    {
        delete audio2midi;
        audio2midi = nullptr;
    }
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
    if (audio2midi == nullptr)
        return;

    // Check if reload button was pressed
    bool shouldReload = *apvts.getRawParameterValue("reloadAlgorithm") > 0.5f;
    if (shouldReload)
    {
        // Reset the button parameter back to false
        apvts.getParameter("reloadAlgorithm")->setValueNotifyingHost(0.0f);
        reloadAlgorithm();
        return;
    }

    // // Configure the algorithm with new parameter values
    // audio2midi->configure("minFrequency",
    //                       minFrequency,
    //                       "maxFrequency",
    //                       maxFrequency,
    //                       "tuningFrequency",
    //                       static_cast<int>(tuningFrequency),
    //                       "pitchConfidenceThreshold",
    //                       pitchConfidenceThreshold,
    //                       "loudnessThreshold",
    //                       loudnessThreshold,
    //                       "transpositionAmount",
    //                       transpositionAmount,
    //                       "applyTimeCompensation",
    //                       applyTimeCompensation);
}

void AudioPluginAudioProcessor::reloadAlgorithm()
{
    // Properly clean up current algorithm
    if (audio2midi != nullptr)
    {
        delete audio2midi;
        audio2midi = nullptr;
    }

    // Clear and prepare input frame
    int currentSamplesPerBlock = static_cast<int>(inputFrame.size());
    if (currentSamplesPerBlock <= 0)
    {
        currentSamplesPerBlock = 512; // fallback to reasonable default
    }
    double currentSampleRate = mSampleRate;
    if (currentSampleRate <= 0)
    {
        currentSampleRate = 44100.0; // fallback to reasonable default
    }

    inputFrame.clear();
    inputFrame.resize(static_cast<std::size_t>(currentSamplesPerBlock));

    // Create new algorithm with current parameters
    if (createAudio2MidiAlgorithm(currentSamplesPerBlock, currentSampleRate, true))
    {
        DBG("Audio2Midi algorithm reloaded successfully");
    }
    else
    {
        DBG("Failed to reload Audio2Midi algorithm");
    }
}

bool AudioPluginAudioProcessor::createAudio2MidiAlgorithm(int    samplesPerBlock,
                                                          double sampleRate,
                                                          bool   useCurrentParameters)
{
    auto& factory = essentia::standard::AlgorithmFactory::instance();

    try
    {
        if (useCurrentParameters)
        {
            // Get current parameter values
            float minFreq = juce::jlimit(20.0f, 20000.0f, (float) *apvts.getRawParameterValue("minFrequency"));
            float maxFreq =
                juce::jlimit(minFreq + 10.0f, 20000.0f, (float) *apvts.getRawParameterValue("maxFrequency"));
            float tuningFreq      = *apvts.getRawParameterValue("tuningFrequency");
            float pitchConfidence = *apvts.getRawParameterValue("pitchConfidenceThreshold");
            float loudnessThresh  = *apvts.getRawParameterValue("loudnessThreshold");
            int   transposition   = static_cast<int>(*apvts.getRawParameterValue("transpositionAmount"));
            bool  applyTimeComp   = *apvts.getRawParameterValue("applyTimeCompensation") > 0.5f;

            // Create algorithm with all parameters
            audio2midi = factory.create("Audio2Midi",
                                        "hopSize",
                                        samplesPerBlock,
                                        "sampleRate",
                                        static_cast<int>(sampleRate),
                                        "minFrequency",
                                        minFreq,
                                        "maxFrequency",
                                        maxFreq,
                                        "tuningFrequency",
                                        static_cast<int>(tuningFreq),
                                        "pitchConfidenceThreshold",
                                        pitchConfidence,
                                        "loudnessThreshold",
                                        loudnessThresh,
                                        "transpositionAmount",
                                        transposition,
                                        "applyTimeCompensation",
                                        applyTimeComp);
        }
        else
        {
            // Create algorithm with default parameters (for initial setup)
            audio2midi = factory.create("Audio2Midi", "hopSize", samplesPerBlock, "sampleRate", sampleRate);
        }

        // Connect inputs and outputs
        audio2midi->input("frame").set(inputFrame);
        audio2midi->output("pitch").set(pitch);
        audio2midi->output("loudness").set(rms);
        audio2midi->output("messageType").set(messageType);
        audio2midi->output("midiNoteNumber").set(midiNoteNumber);
        audio2midi->output("timeCompensation").set(timeCompensation);

        return true;
    }
    catch (const std::exception& e)
    {
        DBG("Failed to create Audio2Midi algorithm: " + juce::String(e.what()));
        audio2midi = nullptr;
        return false;
    }
    catch (...)
    {
        DBG("Failed to create Audio2Midi algorithm: unknown exception");
        audio2midi = nullptr;
        return false;
    }
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages, buffer);
    updateParameters();

    // clear the output before adding stuff
    midiMessages.clear();

    // timestamp implementation from
    // https://forum.juce.com/t/how-to-timestamp-midi-events-in-processblock-when-not-playing/11556/8
    // ask the host for the current time so we can display it...
    juce::AudioPlayHead::CurrentPositionInfo newTime;

    // TODO: use getPosition() instead
    if (getPlayHead() != nullptr && getPlayHead()->getCurrentPosition(newTime))
    {
        // Successfully got the current time from the host..
        lastPosInfo = newTime;
    }
    else
    {
        // If the host fails to fill-in the current time, we'll just clear it to a default..
        lastPosInfo.resetToDefault();
    }

    // keep track of time even whe the host isn't playing (set currentTime = 0 in prepareToPlay)
    currentTime += msPerSample * buffer.getNumSamples();
    if (newTime.isPlaying != true)
        currentTime = 0;
    // DBG("Current time: " + juce::String(currentTime));

    // TODO: estimate RMS and define a threshold for signal content detection
    // TODO: if there is no signal and the system is stopped skip the processing
    // processing preparation
    float*             start = buffer.getWritePointer(0); // get the pointer to the first sample of the first channel
    int                numSamples = buffer.getNumSamples();
    std::vector<float> audio_buffer_vec(start, start + numSamples);
    // TODO: look at how to load data directly from the JUCE buffer
    for (auto sample : audio_buffer_vec)
    {
        inputFrame.push_back(sample);
    }

    // Safety check before calling compute
    if (audio2midi == nullptr)
    {
        DBG("Audio2Midi algorithm is null, skipping processing");
        inputFrame.clear();
        return;
    }

    try
    {
        audio2midi->compute();
    }
    catch (const std::exception& e)
    {
        DBG("Audio2Midi compute failed: " + juce::String(e.what()));
        inputFrame.clear();
        return;
    }
    catch (...)
    {
        DBG("Audio2Midi compute failed with unknown exception");
        inputFrame.clear();
        return;
    }

    // TODO: check the issue we have with MIDI noise and the high performance it needs for small
    // blocks this is just to provide output when it is playing and mute when it is stopped
    if (newTime.isPlaying == true)
    {

        // TODO: use rms value and threshold to silence unvoiced messages
        // TODO: define loudness threshold as parameter and control it with an slider
        // TODO: define pitch confidence threshold as parameter and control it with an slider
        // TODO: apply time compensation for midi events if needed
        // TODO: get velocity value from audio2MIDI

        // filter empty message outputs
        if (messageType.empty())
            return;

        // TODO: convert RMS to velocity
        DBG("rms: " + juce::String(rms) + " - " + juce::String(messageType.size()));

        // process a single message
        if (messageType.size() == 1)
        {

            /*DBG("msg: " + juce::String(messageType[0]) + " - midinote: " +
             * juce::String(midiNoteNumber[0]) + " - timestamp: " + juce::String(currentTime /
             * 1000));*/

            auto message = juce::MidiMessage::noteOn(10, static_cast<int>(midiNoteNumber[0]), (juce::uint8) 100);

            if (messageType[0] == "note_off")
            {
                // generate noteoff message
                message = juce::MidiMessage::noteOff(10, static_cast<int>(midiNoteNumber[0]), (juce::uint8) 100);
            }

            message.setTimeStamp(currentTime);
            auto sampleNumber = (int) (currentTime * mSampleRate);
            midiMessages.addEvent(message, sampleNumber);

            // process note-off and note-on message simultaneously in the same block
        }
        else if (messageType.size() == 2)
        {
            // generate noteoff message
            /*DBG("msg: " + juce::String(messageType[0]) + " - midinote: " +
             * juce::String(midiNoteNumber[0]) + " - timestamp: " + juce::String(currentTime /
             * 1000));*/
            auto message = juce::MidiMessage::noteOff(10, static_cast<int>(midiNoteNumber[0]), (juce::uint8) 100);
            message.setTimeStamp(currentTime);
            auto sampleNumber = (int) (currentTime * mSampleRate);
            midiMessages.addEvent(message, sampleNumber);

            // generate noteon message
            /*DBG("msg2: "+ juce::String(messageType[1]) + " - midinote: " +
             * juce::String(midiNoteNumber[1]) + " - timestamp: " + juce::String(currentTime / 1000)
             * );*/
            message = juce::MidiMessage::noteOn(10, static_cast<int>(midiNoteNumber[1]), (juce::uint8) 100);
            message.setTimeStamp(currentTime);
            sampleNumber = (int) (currentTime * mSampleRate);
            midiMessages.addEvent(message, sampleNumber);
        }
    }

    // clear the block data
    inputFrame.clear();
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor(*this);
    // return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
