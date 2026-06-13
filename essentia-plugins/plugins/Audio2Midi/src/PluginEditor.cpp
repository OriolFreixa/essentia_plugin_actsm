#include "Audio2Midi/PluginEditor.h"

//==============================================================================
// MidiVisualizationComponent Implementation
//==============================================================================
MidiVisualizationComponent::MidiVisualizationComponent()
{
    startTimerHz(30); // Update display 30 times per second
}

MidiVisualizationComponent::~MidiVisualizationComponent()
{
    stopTimer();
}

void MidiVisualizationComponent::paint(juce::Graphics& g)
{
    // Dark background with subtle border (matching waveform style)
    g.setColour(juce::Colour(0xff1a202c));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 8.0f);

    g.setColour(juce::Colour(0xff2d3748));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(1), 8.0f, 1.0f);

    // Placeholder text for now
    g.setColour(juce::Colour(0xff718096));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("MIDI Output Visualization", getLocalBounds(), juce::Justification::centred);
}

void MidiVisualizationComponent::resized()
{
    // Component automatically handles resizing
}

void MidiVisualizationComponent::triggerMidiNote(int noteNumber, float velocity)
{
    // Placeholder implementation - will be implemented later
    juce::ignoreUnused(noteNumber, velocity);
    notesNeedUpdate = true;
}

void MidiVisualizationComponent::timerCallback()
{
    // Placeholder - repaint occasionally for any future animations
    if (notesNeedUpdate.exchange(false))
    {
        repaint();
    }
}

//==============================================================================
// ModernLookAndFeel Implementation
//==============================================================================
ModernLookAndFeel::ModernLookAndFeel()
{
    // Set custom colors (matching the reference)
    setColour(juce::Slider::trackColourId, juce::Colour(0xff2d3748));
    setColour(juce::Slider::thumbColourId, juce::Colour(0xff4299e1));
    setColour(juce::Label::textColourId, juce::Colours::white);
    setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4299e1));
    setColour(juce::TextButton::textColourOffId, juce::Colours::white);
}

void ModernLookAndFeel::drawLinearSlider(juce::Graphics&                 g,
                                         int                             x,
                                         int                             y,
                                         int                             width,
                                         int                             height,
                                         float                           sliderPos,
                                         float                           minSliderPos,
                                         float                           maxSliderPos,
                                         const juce::Slider::SliderStyle style,
                                         juce::Slider&                   slider)
{
    juce::ignoreUnused(minSliderPos, maxSliderPos, style, slider);

    const float trackY      = y + height * 0.5f - 3.0f;
    const float trackHeight = 6.0f;

    // Draw track background with solid color
    g.setColour(juce::Colour(0xff2d3748));
    g.fillRoundedRectangle(x, trackY, width, trackHeight, 3.0f);

    // Draw filled portion with solid color
    const float filledWidth = sliderPos - x;
    if (filledWidth > 0)
    {
        g.setColour(juce::Colour(0xff4299e1));
        g.fillRoundedRectangle(x, trackY, filledWidth, trackHeight, 3.0f);
    }

    drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
}

void ModernLookAndFeel::drawLinearSliderThumb(juce::Graphics&                 g,
                                              int                             x,
                                              int                             y,
                                              int                             width,
                                              int                             height,
                                              float                           sliderPos,
                                              float                           minSliderPos,
                                              float                           maxSliderPos,
                                              const juce::Slider::SliderStyle style,
                                              juce::Slider&                   slider)
{
    juce::ignoreUnused(x, y, width, height, minSliderPos, maxSliderPos, style, slider);

    const float thumbSize = 20.0f;
    const float thumbX    = sliderPos - thumbSize * 0.5f;
    const float thumbY    = y + height * 0.5f - thumbSize * 0.5f;

    // Draw thumb with solid color
    g.setColour(juce::Colour(0xff4299e1));
    g.fillEllipse(thumbX, thumbY, thumbSize, thumbSize);

    // Add simple border
    g.setColour(juce::Colour(0xff3182ce));
    g.drawEllipse(thumbX, thumbY, thumbSize, thumbSize, 2.0f);
}

void ModernLookAndFeel::drawToggleButton(juce::Graphics&     g,
                                         juce::ToggleButton& button,
                                         bool                shouldDrawButtonAsHighlighted,
                                         bool                shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

    const auto  bounds       = button.getLocalBounds().toFloat();
    const float checkboxSize = 18.0f;
    const float checkboxX    = bounds.getX();
    const float checkboxY    = bounds.getCentreY() - checkboxSize * 0.5f;

    // Draw checkbox background
    g.setColour(button.getToggleState() ? juce::Colour(0xff4299e1) : juce::Colour(0xff2d3748));
    g.fillRoundedRectangle(checkboxX, checkboxY, checkboxSize, checkboxSize, 4.0f);

    // Draw checkbox border
    g.setColour(juce::Colour(0xff4a5568));
    g.drawRoundedRectangle(checkboxX, checkboxY, checkboxSize, checkboxSize, 4.0f, 2.0f);

    // Draw checkmark if toggled
    if (button.getToggleState())
    {
        g.setColour(juce::Colours::white);
        g.setFont(juce::FontOptions(12.0f));
        g.drawText("✓", checkboxX, checkboxY, checkboxSize, checkboxSize, juce::Justification::centred);
    }
}

//==============================================================================
// AudioPluginAudioProcessorEditor Implementation
//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
{
    // Set custom look and feel
    setLookAndFeel(&modernLF);

    // Setup title and subtitle
    titleLabel.setText("Audio2Midi", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4299e1));
    titleLabel.setJustificationType(juce::Justification::centred);
    // Allow mouse clicks to pass through to the editor
    titleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText("Real-time Audio to MIDI Conversion", juce::dontSendNotification);
    subtitleLabel.setFont(juce::FontOptions(14.0f));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    // Setup group labels
    frequencyGroupLabel.setText("FREQUENCY RANGE", juce::dontSendNotification);
    frequencyGroupLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    frequencyGroupLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4299e1));
    frequencyGroupLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(frequencyGroupLabel);

    tuningGroupLabel.setText("TUNING & DETECTION", juce::dontSendNotification);
    tuningGroupLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    tuningGroupLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4299e1));
    tuningGroupLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(tuningGroupLabel);

    midiGroupLabel.setText("MIDI PROCESSING", juce::dontSendNotification);
    midiGroupLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    midiGroupLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4299e1));
    midiGroupLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(midiGroupLabel);

    systemGroupLabel.setText("SYSTEM", juce::dontSendNotification);
    systemGroupLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    systemGroupLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4299e1));
    systemGroupLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(systemGroupLabel);

    // Setup frequency range controls
    setupSlider(minFrequencySlider, minFrequencyLabel, minFrequencyValueLabel, "Min Frequency", "minFrequency");
    setupSlider(maxFrequencySlider, maxFrequencyLabel, maxFrequencyValueLabel, "Max Frequency", "maxFrequency");

    // Setup tuning & detection controls
    setupSlider(tuningFrequencySlider,
                tuningFrequencyLabel,
                tuningFrequencyValueLabel,
                "Tuning Frequency",
                "tuningFrequency");
    setupSlider(pitchConfidenceSlider,
                pitchConfidenceLabel,
                pitchConfidenceValueLabel,
                "Pitch Confidence",
                "pitchConfidenceThreshold");
    setupSlider(loudnessThresholdSlider,
                loudnessThresholdLabel,
                loudnessThresholdValueLabel,
                "Loudness Threshold",
                "loudnessThreshold");

    // Setup MIDI processing controls
    setupSlider(transpositionSlider,
                transpositionLabel,
                transpositionValueLabel,
                "Transposition",
                "transpositionAmount");

    // Setup toggle button
    timeCompensationToggle.setButtonText("Apply Time Compensation");
    timeCompensationToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(timeCompensationToggle);

    timeCompensationLabel.setText("Time Compensation", juce::dontSendNotification);
    timeCompensationLabel.setFont(juce::FontOptions(13.0f));
    timeCompensationLabel.setColour(juce::Label::textColourId, juce::Colour(0xffccd7e5));
    timeCompensationLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(timeCompensationLabel);

    // Setup reload button
    reloadButton.setButtonText("RELOAD ALGORITHM");
    reloadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff48bb78)); // Green by default
    reloadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addAndMakeVisible(reloadButton);

    // Setup MIDI visualization
    addAndMakeVisible(midiVisualization);

    // Setup parameter attachments
    minFrequencyAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.getAPVTS(),
                                                                                          "minFrequency",
                                                                                          minFrequencySlider));
    maxFrequencyAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.getAPVTS(),
                                                                                          "maxFrequency",
                                                                                          maxFrequencySlider));
    tuningFrequencyAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.getAPVTS(),
                                                                                             "tuningFrequency",
                                                                                             tuningFrequencySlider));
    pitchConfidenceAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.getAPVTS(),
                                                                                             "pitchConfidenceThreshold",
                                                                                             pitchConfidenceSlider));
    loudnessThresholdAttachment.reset(
        new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.getAPVTS(),
                                                                 "loudnessThreshold",
                                                                 loudnessThresholdSlider));
    transpositionAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(processorRef.getAPVTS(),
                                                                                           "transpositionAmount",
                                                                                           transpositionSlider));
    timeCompensationAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(processorRef.getAPVTS(),
                                                                                              "applyTimeCompensation",
                                                                                              timeCompensationToggle));

    // Handle reload button click manually instead of using ButtonAttachment
    reloadButton.onClick = [this]() {
        // Change button appearance to show it's processing
        reloadButton.setButtonText("UPDATING...");
        reloadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff718096)); // Gray
        reloadButton.setEnabled(false); // Disable button during processing

        // Set the parameter to true to trigger reload
        processorRef.getAPVTS().getParameter("reloadAlgorithm")->setValueNotifyingHost(1.0f);

        isReloading = true;
    };

    // Load logos from binary data (if available)
    upfLogo      = juce::ImageCache::getFromMemory(BinaryData::upflogo_png, BinaryData::upflogo_pngSize);
    essentiaLogo = juce::ImageCache::getFromMemory(BinaryData::essentia_logo_png, BinaryData::essentia_logo_pngSize);

    // Setup powered by label
    poweredByLabel.setText("powered by", juce::dontSendNotification);
    poweredByLabel.setFont(juce::FontOptions(8.30437f));
    poweredByLabel.setColour(juce::Label::textColourId, juce::Colour(0xff718096));
    poweredByLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(poweredByLabel);

    // Set plugin size
    setSize(600, 780);
    setResizable(false, false);

    // Start timer for updates
    startTimerHz(30);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::setupSlider(juce::Slider&       slider,
                                                  juce::Label&        label,
                                                  juce::Label&        valueLabel,
                                                  const juce::String& labelText,
                                                  const juce::String& parameterID)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setFont(juce::FontOptions(13.0f));
    label.setColour(juce::Label::textColourId, juce::Colour(0xffccd7e5));
    label.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(label);

    valueLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    valueLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4299e1));
    valueLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(valueLabel);

    // Setup value change callback
    slider.onValueChange = [this, &slider, &valueLabel, parameterID]() {
        float value = static_cast<float>(slider.getValue());

        if (parameterID == "minFrequency" || parameterID == "maxFrequency")
            updateValueLabel(valueLabel, value, " Hz");
        else if (parameterID == "tuningFrequency")
            updateValueLabel(valueLabel, value, " Hz", 1);
        else if (parameterID == "pitchConfidenceThreshold")
            updateValueLabel(valueLabel, value, "", 2);
        else if (parameterID == "loudnessThreshold")
            updateValueLabel(valueLabel, value, " dB", 1);
        else if (parameterID == "transpositionAmount")
            updateValueLabel(valueLabel, value, " Semitones");
    };
}

void AudioPluginAudioProcessorEditor::updateValueLabel(juce::Label&        valueLabel,
                                                       float               value,
                                                       const juce::String& unit,
                                                       int                 decimals)
{
    juce::String valueText = juce::String(value, decimals) + unit;
    valueLabel.setText(valueText, juce::dontSendNotification);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    // Update all value labels
    updateValueLabel(minFrequencyValueLabel, static_cast<float>(minFrequencySlider.getValue()), " Hz");
    updateValueLabel(maxFrequencyValueLabel, static_cast<float>(maxFrequencySlider.getValue()), " Hz");
    updateValueLabel(tuningFrequencyValueLabel, static_cast<float>(tuningFrequencySlider.getValue()), " Hz", 1);
    updateValueLabel(pitchConfidenceValueLabel, static_cast<float>(pitchConfidenceSlider.getValue()), "", 2);
    updateValueLabel(loudnessThresholdValueLabel, static_cast<float>(loudnessThresholdSlider.getValue()), " dB", 1);
    updateValueLabel(transpositionValueLabel, static_cast<float>(transpositionSlider.getValue()), " Semitones");

    // Check if reload is complete and restore button appearance
    if (isReloading)
    {
        // Check if the parameter is back to false (reload complete)
        bool reloadParam = *processorRef.getAPVTS().getRawParameterValue("reloadAlgorithm") > 0.5f;
        if (!reloadParam)
        {
            // Reload is complete, restore button appearance
            reloadButton.setButtonText("RELOAD ALGORITHM");
            reloadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff48bb78)); // Green
            reloadButton.setEnabled(true);
            isReloading = false;
        }
    }
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Solid background
    g.setColour(juce::Colour(0xff1a1f2e));
    g.fillAll();

    // Draw main container with solid color and border
    const auto containerBounds = getLocalBounds().reduced(20).toFloat();
    g.setColour(juce::Colour(0xff2a3441));
    g.fillRoundedRectangle(containerBounds, 16.0f);

    g.setColour(juce::Colour(0xff3a4553));
    g.drawRoundedRectangle(containerBounds, 16.0f, 1.0f);

    // Draw group backgrounds with proportional positioning
    const float groupMargin = 40.0f;
    const float groupRadius = 8.0f;

    auto drawGroupBackground = [&](float y, float height) {
        juce::Rectangle<float> groupBounds(containerBounds.getX() + groupMargin,
                                           y,
                                           containerBounds.getWidth() - 2 * groupMargin,
                                           height);

        g.setColour(juce::Colour(0xff1a202c).withAlpha(0.5f));
        g.fillRoundedRectangle(groupBounds, groupRadius);

        g.setColour(juce::Colour(0xff3a4553));
        g.drawRoundedRectangle(groupBounds, groupRadius, 1.0f);
    };

    // Draw group backgrounds with corrected positions and heights
    drawGroupBackground(93.0f, 100.0f);  // Frequency range
    drawGroupBackground(200.0f, 230.0f); // Tuning & detection
    drawGroupBackground(440.0f, 130.0f); // MIDI processing
    drawGroupBackground(580.0f, 75.0f);  // System

    // Footer layout with logos - MUCH more space at bottom
    const float footerBaseline     = 50.0f; // Increased significantly
    const float footerSideMargin   = 20.0f;
    const float standardLogoHeight = 38.0f;

    // Draw UPF logo (left side)
    if (upfLogo.isValid())
    {
        const float upfLogoWidth = upfLogo.getWidth() * (standardLogoHeight / upfLogo.getHeight());
        const int   upfLogoX     = static_cast<int>(containerBounds.getX() + footerSideMargin);
        const int   upfLogoY     = static_cast<int>(containerBounds.getBottom() - footerBaseline - standardLogoHeight);

        g.drawImage(upfLogo,
                    upfLogoX,
                    upfLogoY,
                    static_cast<int>(upfLogoWidth),
                    static_cast<int>(standardLogoHeight),
                    0,
                    0,
                    upfLogo.getWidth(),
                    upfLogo.getHeight());
    }

    // Draw Essentia logo (right side)
    if (essentiaLogo.isValid())
    {
        const float essentiaLogoWidth = essentiaLogo.getWidth() * (standardLogoHeight / essentiaLogo.getHeight());
        const int   essentiaLogoX = static_cast<int>(containerBounds.getRight() - footerSideMargin - essentiaLogoWidth);
        const int   essentiaLogoY = static_cast<int>(containerBounds.getBottom() - footerBaseline - standardLogoHeight);

        g.drawImage(essentiaLogo,
                    essentiaLogoX,
                    essentiaLogoY,
                    static_cast<int>(essentiaLogoWidth),
                    static_cast<int>(standardLogoHeight),
                    0,
                    0,
                    essentiaLogo.getWidth(),
                    essentiaLogo.getHeight());
    }
}

void AudioPluginAudioProcessorEditor::resized()
{
    const int totalWidth        = getWidth();
    const int totalHeight       = getHeight();
    const int margin            = 60;
    const int sliderHeight      = 25;
    const int labelHeight       = 20;
    const int valueWidth        = 80;
    const int groupTitleSpacing = 10;
    const int controlSpacing    = 6;
    const int sectionSpacing    = 25; // Space between sections

    // Header Section
    titleLabel.setBounds(0, 25, totalWidth, 30);
    subtitleLabel.setBounds(0, 55, totalWidth, 20);

    int currentY = 100;

    // Section 1: Frequency Range
    frequencyGroupLabel.setBounds(margin, currentY, 200, labelHeight);
    currentY += labelHeight + groupTitleSpacing;

    // 2-column grid for frequency controls
    const int columnWidth = (totalWidth - 2 * margin - 20) / 2;
    const int column2X    = margin + columnWidth + 20;

    // Min Frequency (Left column)
    minFrequencyLabel.setBounds(margin, currentY, 120, labelHeight);
    minFrequencyValueLabel.setBounds(margin + columnWidth - valueWidth, currentY, valueWidth, labelHeight);
    minFrequencySlider.setBounds(margin, currentY + labelHeight + controlSpacing, columnWidth, sliderHeight);

    // Max Frequency (Right column)
    maxFrequencyLabel.setBounds(column2X, currentY, 120, labelHeight);
    maxFrequencyValueLabel.setBounds(column2X + columnWidth - valueWidth, currentY, valueWidth, labelHeight);
    maxFrequencySlider.setBounds(column2X, currentY + labelHeight + controlSpacing, columnWidth, sliderHeight);

    currentY += labelHeight + controlSpacing + sliderHeight + sectionSpacing;

    // Section 2: Tuning & Detection
    tuningGroupLabel.setBounds(margin, currentY, 200, labelHeight);
    currentY += labelHeight + groupTitleSpacing;

    // Tuning Frequency
    tuningFrequencyLabel.setBounds(margin, currentY, 140, labelHeight);
    tuningFrequencyValueLabel.setBounds(totalWidth - margin - valueWidth, currentY, valueWidth, labelHeight);
    tuningFrequencySlider.setBounds(margin,
                                    currentY + labelHeight + controlSpacing,
                                    totalWidth - 2 * margin,
                                    sliderHeight);
    currentY += labelHeight + controlSpacing + sliderHeight + 15;

    // Pitch Confidence
    pitchConfidenceLabel.setBounds(margin, currentY, 140, labelHeight);
    pitchConfidenceValueLabel.setBounds(totalWidth - margin - valueWidth, currentY, valueWidth, labelHeight);
    pitchConfidenceSlider.setBounds(margin,
                                    currentY + labelHeight + controlSpacing,
                                    totalWidth - 2 * margin,
                                    sliderHeight);
    currentY += labelHeight + controlSpacing + sliderHeight + 15;

    // Loudness Threshold
    loudnessThresholdLabel.setBounds(margin, currentY, 140, labelHeight);
    loudnessThresholdValueLabel.setBounds(totalWidth - margin - valueWidth, currentY, valueWidth, labelHeight);
    loudnessThresholdSlider.setBounds(margin,
                                      currentY + labelHeight + controlSpacing,
                                      totalWidth - 2 * margin,
                                      sliderHeight);
    currentY += labelHeight + controlSpacing + sliderHeight + sectionSpacing;

    // Section 3: MIDI Processing
    midiGroupLabel.setBounds(margin, currentY, 200, labelHeight);
    currentY += labelHeight + groupTitleSpacing;

    // Transposition
    transpositionLabel.setBounds(margin, currentY, 140, labelHeight);
    transpositionValueLabel.setBounds(totalWidth - margin - valueWidth, currentY, valueWidth, labelHeight);
    transpositionSlider.setBounds(margin,
                                  currentY + labelHeight + controlSpacing,
                                  totalWidth - 2 * margin,
                                  sliderHeight);
    currentY += labelHeight + controlSpacing + sliderHeight + 15;

    // Time Compensation
    timeCompensationLabel.setBounds(margin, currentY, 140, labelHeight);
    timeCompensationToggle.setBounds(margin + 200, currentY, 200, labelHeight);
    currentY += labelHeight + sectionSpacing;

    // Section 4: System
    systemGroupLabel.setBounds(margin, currentY, 200, labelHeight);
    currentY += labelHeight + groupTitleSpacing;

    // Reload Button
    reloadButton.setBounds(margin, currentY, 150, 30);
    currentY += 40;

    // Remove MIDI Visualization to make more room
    midiVisualization.setBounds(0, 0, 0, 0); // Hide it

    // Position "powered by" text at the bottom
    poweredByLabel.setBounds(totalWidth - 120, totalHeight - 45, 80, 12);
}

//==============================================================================
// Easter egg implementation
//==============================================================================
void AudioPluginAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Check if the click is within the title label bounds
    if (titleLabel.getBounds().contains(event.getPosition()))
    {
        showDeveloperInfo();
    }

    // Also call the base class to ensure proper handling
    AudioProcessorEditor::mouseDown(event);
}

void AudioPluginAudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
{
    // Check if mouse is over the title label
    bool wasHovered = titleHovered;
    titleHovered    = titleLabel.getBounds().contains(event.getPosition());

    // Update cursor only (no color change)
    if (titleHovered != wasHovered)
    {
        setMouseCursor(titleHovered ? juce::MouseCursor::PointingHandCursor : juce::MouseCursor::NormalCursor);
    }
}

void AudioPluginAudioProcessorEditor::showDeveloperInfo()
{
    // Use JUCE's built-in showOkCancelBox for better reliability
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::InfoIcon,
        "About the Developer",
        "Plugin developed by Fernando Garcia (github.com/fergarciadlc)\n\nThanks for "
        "using this plugin!\n\nCheckout the source code and contribute:\n\n",
        "MTG/essentia-plugins\n",
        "Cool!",
        nullptr,
        juce::ModalCallbackFunction::create([](int result) {
            if (result == 1) // "Visit GitHub" clicked
            {
                juce::URL("https://github.com/MTG/essentia-plugins").launchInDefaultBrowser();
            }
        }));
}