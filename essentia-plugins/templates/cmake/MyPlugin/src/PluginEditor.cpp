#include "MyPlugin/PluginEditor.h"
#include "MyPlugin/PluginProcessor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    juce::ignoreUnused(processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
    startTimerHz(30);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    repaint();
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::white);
    g.setFont(25.0f);
    g.drawFittedText("Essentia:" + juce::String(essentia::version),
                     getLocalBounds(),
                     juce::Justification::centredTop,
                     1);

    g.setFont(35.0f);

    const float eLin = processorRef.getEnergyLinear();
    const float eDB  = processorRef.getEnergydB();

    juce::String info;
    info << "Energy (lin): " << juce::String(eLin, 3) << "\n"
         << "Energy (dB):  " << juce::String(eDB, 3) << " dB";

    g.drawFittedText(info, getLocalBounds().reduced(10), juce::Justification::centred, 2);

    g.setFont(15.0f);
    g.drawFittedText("plugins/MyPlugin", getLocalBounds(), juce::Justification::centredBottom, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
