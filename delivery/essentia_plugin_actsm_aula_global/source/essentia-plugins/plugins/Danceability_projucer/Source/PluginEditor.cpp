#include "PluginEditor.h"

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
EssentiaPluginAudioProcessorEditor::EssentiaPluginAudioProcessorEditor(EssentiaPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
{
    // Set custom look and feel
    setLookAndFeel(&modernLF);

    // Setup title label
    titleLabel.setText("Danceability Analyzer", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    // Allow mouse clicks to pass through to the editor
    titleLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(titleLabel);

    // Setup subtitle label
    subtitleLabel.setText("Real-time danceability estimation", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font(14.0f));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    // Setup danceability value label
    danceabilityValueLabel.setText("0.000", juce::dontSendNotification);
    danceabilityValueLabel.setFont(juce::Font(56.0f, juce::Font::bold));
    danceabilityValueLabel.setColour(juce::Label::textColourId, juce::Colour(0xffed64a6)); // Pink/magenta
    danceabilityValueLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(danceabilityValueLabel);

    // Setup danceability unit label
    danceabilityUnitLabel.setText("Danceability", juce::dontSendNotification);
    danceabilityUnitLabel.setFont(juce::Font(14.0f));
    danceabilityUnitLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    danceabilityUnitLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(danceabilityUnitLabel);

    minTauLabel.setText("minTau", juce::dontSendNotification);
    minTauLabel.setFont(juce::Font(12.0f));
    minTauLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    minTauLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(minTauLabel);

    maxTauLabel.setText("maxTau", juce::dontSendNotification);
    maxTauLabel.setFont(juce::Font(12.0f));
    maxTauLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    maxTauLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(maxTauLabel);

    tauMultiplierLabel.setText("tauMultiplier", juce::dontSendNotification);
    tauMultiplierLabel.setFont(juce::Font(12.0f));
    tauMultiplierLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    tauMultiplierLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(tauMultiplierLabel);

    analysisWindowLabel.setText("window (s)", juce::dontSendNotification);
    analysisWindowLabel.setFont(juce::Font(12.0f));
    analysisWindowLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa0aec0));
    analysisWindowLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(analysisWindowLabel);

    minTauSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    minTauSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
    addAndMakeVisible(minTauSlider);

    maxTauSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    maxTauSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
    addAndMakeVisible(maxTauSlider);

    tauMultiplierSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    tauMultiplierSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
    addAndMakeVisible(tauMultiplierSlider);

    analysisWindowSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    analysisWindowSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 20);
    addAndMakeVisible(analysisWindowSlider);

    minTauAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "minTau", minTauSlider);
    maxTauAttachment = std::make_unique<SliderAttachment>(processorRef.apvts, "maxTau", maxTauSlider);
    tauMultiplierAttachment = std::make_unique<SliderAttachment>(processorRef.apvts,
                                                                 "tauMultiplier",
                                                                 tauMultiplierSlider);
    analysisWindowAttachment = std::make_unique<SliderAttachment>(processorRef.apvts,
                                                                  "analysisWindowSeconds",
                                                                  analysisWindowSlider);

    // Load logos from binary data
    upfLogo      = juce::ImageCache::getFromMemory(BinaryData::upflogo_png, BinaryData::upflogo_pngSize);
    essentiaLogo = juce::ImageCache::getFromMemory(BinaryData::essentia_logo_png, BinaryData::essentia_logo_pngSize);

    // Setup "powered by" label
    poweredByLabel.setText("powered by", juce::dontSendNotification);
    poweredByLabel.setFont(juce::Font(8.30437f));
    poweredByLabel.setColour(juce::Label::textColourId, juce::Colour(0xff718096));
    poweredByLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(poweredByLabel);

    // Set plugin size (fixed, not resizable) - compact and focused
    setSize(400, 470);
    setResizable(false, false);

    startTimerHz(30);
}

EssentiaPluginAudioProcessorEditor::~EssentiaPluginAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void EssentiaPluginAudioProcessorEditor::timerCallback()
{
    // Update danceability value display
    const float danceability = processorRef.getDanceability();

    // Update labels
    danceabilityValueLabel.setText(juce::String(danceability, 3), juce::dontSendNotification);

    repaint();
}

void EssentiaPluginAudioProcessorEditor::paint(juce::Graphics& g)
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

    // Draw value display background with more prominent styling
    const auto danceabilityValueBounds = getLocalBounds()
                                       .reduced(30, 0)
                                       .withY(static_cast<int>(getHeight() * 0.25f))
                                       .withHeight(static_cast<int>(getHeight() * 0.22f))
                                       .withWidth(getWidth() - 60)
                                       .toFloat();

    // Danceability background
    g.setColour(juce::Colour(0xff1a202c));
    g.fillRoundedRectangle(danceabilityValueBounds, 12.0f);
    g.setColour(juce::Colour(0xffed64a6).withAlpha(0.3f)); // Pink accent
    g.drawRoundedRectangle(danceabilityValueBounds, 12.0f, 2.0f);

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

void EssentiaPluginAudioProcessorEditor::resized()
{
    // Fixed layout values for cleaner, more focused layout
    const float titleY         = 0.08f;
    const float subtitleY      = 0.16f;
    const float danceabilityValueY = 0.28f;
    const float danceabilityUnitY  = 0.43f;
    const float footerBaseline = 20.0f;
    const float footerTextGap  = 5.4f;

    // Use setBoundsRelative for scalable layout
    titleLabel.setBoundsRelative(0.0f, titleY, 1.0f, 0.08f);
    subtitleLabel.setBoundsRelative(0.0f, subtitleY, 1.0f, 0.05f);

    // Larger, more prominent value display
    danceabilityValueLabel.setBoundsRelative(0.1f, danceabilityValueY, 0.8f, 0.12f);
    danceabilityUnitLabel.setBoundsRelative(0.1f, danceabilityUnitY, 0.8f, 0.04f);

    minTauLabel.setBoundsRelative(0.10f, 0.51f, 0.28f, 0.05f);
    minTauSlider.setBoundsRelative(0.34f, 0.51f, 0.56f, 0.05f);
    maxTauLabel.setBoundsRelative(0.10f, 0.59f, 0.28f, 0.05f);
    maxTauSlider.setBoundsRelative(0.34f, 0.59f, 0.56f, 0.05f);
    tauMultiplierLabel.setBoundsRelative(0.10f, 0.67f, 0.28f, 0.05f);
    tauMultiplierSlider.setBoundsRelative(0.34f, 0.67f, 0.56f, 0.05f);
    analysisWindowLabel.setBoundsRelative(0.10f, 0.75f, 0.28f, 0.05f);
    analysisWindowSlider.setBoundsRelative(0.34f, 0.75f, 0.56f, 0.05f);

    // Position "powered by" text above Essentia logo
    const float logoHeight   = 32.0f; // Compact for the layout
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
void EssentiaPluginAudioProcessorEditor::mouseDown(const juce::MouseEvent& event)
{
    // Check if the click is within the title label bounds
    if (titleLabel.getBounds().contains(event.getPosition()))
    {
        showDeveloperInfo();
    }

    // Also call the base class to ensure proper handling
    AudioProcessorEditor::mouseDown(event);
}

void EssentiaPluginAudioProcessorEditor::mouseMove(const juce::MouseEvent& event)
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

void EssentiaPluginAudioProcessorEditor::showDeveloperInfo()
{
    // Use JUCE's built-in showOkCancelBox for better reliability
    juce::AlertWindow::showOkCancelBox(
        juce::AlertWindow::InfoIcon,
        "About the plugin",
        "Thanks for using this plugin!\n\nCheckout the source code and contribute:\n\n",
        "MTG/essentia-plugins\n",
        "",
        nullptr,
        juce::ModalCallbackFunction::create([](int result) {
            if (result == 1) // "Visit GitHub" clicked
            {
                juce::URL("https://github.com/MTG/essentia-plugins").launchInDefaultBrowser();
            }
        }));
}
