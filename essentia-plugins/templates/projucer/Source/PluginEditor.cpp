/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VsttemplateAudioProcessorEditor::VsttemplateAudioProcessorEditor (VsttemplateAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
    startTimerHz(30);
}

VsttemplateAudioProcessorEditor::~VsttemplateAudioProcessorEditor()
{
}

void VsttemplateAudioProcessorEditor::timerCallback()
{
    repaint(); // Triggers the paint() method
}

//==============================================================================
void VsttemplateAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (45.0f);

    g.drawFittedText ("JUCE + Essentia", getLocalBounds(), juce::Justification::centredTop, 1);
    
    g.setFont (35.0f);

    float rms = audioProcessor.getRMS();
    float energy = audioProcessor.getEnergy();

    juce::String info;
    info << "RMS: " << juce::String(rms, 3) << "\n";
    info << "Energy: " << juce::String(energy, 3);

    g.drawFittedText(info, getLocalBounds().reduced(10), juce::Justification::centred, 2);
}

void VsttemplateAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
