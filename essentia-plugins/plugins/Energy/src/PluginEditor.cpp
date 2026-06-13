#include "Energy/PluginEditor.h"

//==============================================================================
// ModernLookAndFeel Implementation
//==============================================================================
ModernLookAndFeel::ModernLookAndFeel()
{
    // Set custom colors
    setColour(juce::Label::textColourId, juce::Colours::white);
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

    // Setup title label
    titleLabel.setText("Energy Analyzer", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    // Allow mouse clicks to pass through to the editor
    titleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(titleLabel);

    // Setup subtitle label
    subtitleLabel.setText("Real-time audio energy analysis", juce::dontSendNotification);
    subtitleLabel.setFont(juce::FontOptions(14.0f));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    // Setup Energy Linear value label
    energyLinearValueLabel.setText("0.000", juce::dontSendNotification);
    energyLinearValueLabel.setFont(juce::FontOptions(48.0f, juce::Font::bold));
    energyLinearValueLabel.setColour(juce::Label::textColourId, juce::Colour(0xff68d391));
    energyLinearValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(energyLinearValueLabel);

    // Setup Energy Linear unit label
    energyLinearUnitLabel.setText("Linear Energy", juce::dontSendNotification);
    energyLinearUnitLabel.setFont(juce::FontOptions(14.0f));
    energyLinearUnitLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    energyLinearUnitLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(energyLinearUnitLabel);

    // Setup Energy dB value label
    energyDbValueLabel.setText("-60.0 dB", juce::dontSendNotification);
    energyDbValueLabel.setFont(juce::FontOptions(48.0f, juce::Font::bold));
    energyDbValueLabel.setColour(juce::Label::textColourId, juce::Colour(0xff63b3ed));
    energyDbValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(energyDbValueLabel);

    // Setup Energy dB unit label
    energyDbUnitLabel.setText("Energy (dB)", juce::dontSendNotification);
    energyDbUnitLabel.setFont(juce::FontOptions(14.0f));
    energyDbUnitLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    energyDbUnitLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(energyDbUnitLabel);

    // Load logos from binary data
    upfLogo      = juce::ImageCache::getFromMemory(BinaryData::upflogo_png, BinaryData::upflogo_pngSize);
    essentiaLogo = juce::ImageCache::getFromMemory(BinaryData::essentia_logo_png, BinaryData::essentia_logo_pngSize);

    // Setup "powered by" label
    poweredByLabel.setText("powered by", juce::dontSendNotification);
    poweredByLabel.setFont(juce::FontOptions(8.30437f));
    poweredByLabel.setColour(juce::Label::textColourId, juce::Colour(0xff718096));
    poweredByLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(poweredByLabel);

    // Set plugin size (fixed, not resizable) - smaller and more focused
    setSize(400, 350);
    setResizable(false, false);

    startTimerHz(30);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void AudioPluginAudioProcessorEditor::timerCallback()
{
    // Update energy value displays
    const float eLin = processorRef.getEnergyLinear();
    const float eDB  = processorRef.getEnergydB();

    // Update labels
    energyLinearValueLabel.setText(juce::String(eLin, 3), juce::dontSendNotification);
    energyDbValueLabel.setText(juce::String(eDB, 1) + " dB", juce::dontSendNotification);

    repaint();
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

    // Draw value display backgrounds with more prominent styling
    const auto linearValueBounds = getLocalBounds()
                                       .reduced(30, 0)
                                       .withY(static_cast<int>(getHeight() * 0.25f))
                                       .withHeight(static_cast<int>(getHeight() * 0.25f))
                                       .withWidth(getWidth() - 60)
                                       .toFloat();

    const auto dbValueBounds = getLocalBounds()
                                   .reduced(30, 0)
                                   .withY(static_cast<int>(getHeight() * 0.50f))
                                   .withHeight(static_cast<int>(getHeight() * 0.22f))
                                   .withWidth(getWidth() - 60)
                                   .toFloat();

    // Linear energy background
    g.setColour(juce::Colour(0xff1a202c));
    g.fillRoundedRectangle(linearValueBounds, 12.0f);
    g.setColour(juce::Colour(0xff38a169).withAlpha(0.3f));
    g.drawRoundedRectangle(linearValueBounds, 12.0f, 2.0f);

    // dB energy background
    g.setColour(juce::Colour(0xff1a202c));
    g.fillRoundedRectangle(dbValueBounds, 12.0f);
    g.setColour(juce::Colour(0xff4299e1).withAlpha(0.3f));
    g.drawRoundedRectangle(dbValueBounds, 12.0f, 2.0f);

    // Footer layout with consistent positioning
    const float footerBaseline     = 20.0f;
    const float footerSideMargin   = 15.0f;
    const float standardLogoHeight = 32.0f;

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

    // Draw Essentia logo (right side, aligned on same baseline)
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
    // Fixed layout values for cleaner, more focused layout
    const float titleY         = 0.08f;
    const float subtitleY      = 0.16f;
    const float linearValueY   = 0.28f;
    const float linearUnitY    = 0.45f;
    const float dbValueY       = 0.52f; // Moved up from 0.58f
    const float dbUnitY        = 0.68f; // Moved up from 0.75f
    const float footerBaseline = 20.0f;
    const float footerTextGap  = 5.4f;

    // Use setBoundsRelative for scalable layout
    titleLabel.setBoundsRelative(0.0f, titleY, 1.0f, 0.08f);
    subtitleLabel.setBoundsRelative(0.0f, subtitleY, 1.0f, 0.05f);

    // Larger, more prominent value displays stacked vertically
    energyLinearValueLabel.setBoundsRelative(0.1f, linearValueY, 0.8f, 0.12f);
    energyLinearUnitLabel.setBoundsRelative(0.1f, linearUnitY, 0.8f, 0.04f);
    energyDbValueLabel.setBoundsRelative(0.1f, dbValueY, 0.8f, 0.12f);
    energyDbUnitLabel.setBoundsRelative(0.1f, dbUnitY, 0.8f, 0.04f);

    // Position "powered by" text above Essentia logo
    const float logoHeight   = 32.0f; // Slightly smaller for the compact layout
    const float logoBaseline = footerBaseline;
    const float textGap      = footerTextGap;

    poweredByLabel.setBounds(getWidth() - 100,
                             static_cast<int>(getHeight() - logoBaseline - logoHeight - textGap - 10),
                             70,
                             10);
}

//==============================================================================
// Easter egg implementation
//==============================================================================
void AudioPluginAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Debug output
    DBG("Mouse clicked at: " << event.getPosition().toString());
    DBG("Title bounds: " << titleLabel.getBounds().toString());

    // Check if the click is within the title label bounds
    if (titleLabel.getBounds().contains(event.getPosition()))
    {
        DBG("Title clicked!");
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
