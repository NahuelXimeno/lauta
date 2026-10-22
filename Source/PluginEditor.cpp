#include "PluginProcessor.h"
#include "PluginEditor.h"

LautaFXAudioProcessorEditor::LautaFXAudioProcessorEditor (LautaFXAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel (&lautaLookAndFeel);

    auto& apvts = audioProcessor.getAPVTS();

    // 1. MACROS
    setupRotary (macroEnergySlider, macroEnergyLabel, "ENERGY", juce::Colour (0xffff6b4a), true);
    energyAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::macroEnergy.getParamID(), macroEnergySlider);

    setupRotary (macroSpaceSlider, macroSpaceLabel, "SPACE", juce::Colour (0xff2dd4bf), true);
    spaceAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::macroSpace.getParamID(), macroSpaceSlider);

    setupRotary (macroMovementSlider, macroMovementLabel, "MOVEMENT", juce::Colour (0xffa3e635), true);
    movementAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::macroMovement.getParamID(), macroMovementSlider);

    setupRotary (macroWildSlider, macroWildLabel, "WILD", juce::Colour (0xffec4899), true);
    wildAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::macroWild.getParamID(), macroWildSlider);

    // 2. FILTER
    addAndMakeVisible (filterEnableBtn);
    filterEnableAttachment = std::make_unique<ButtonAttachment> (apvts, LautaParams::filterEnabled.getParamID(), filterEnableBtn);

    setupRotary (filterCutoffSlider, filterCutoffLabel, "Cutoff", juce::Colour (0xff94a3b8));
    filterCutoffSlider.setTextValueSuffix (" Hz");
    filterCutoffAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::filterCutoff.getParamID(), filterCutoffSlider);

    setupRotary (filterResSlider, filterResLabel, "Res", juce::Colour (0xff94a3b8));
    filterResAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::filterResonance.getParamID(), filterResSlider);

    filterTypeBox.addItemList ({ "LowPass", "HighPass", "BandPass" }, 1);
    addAndMakeVisible (filterTypeBox);
    filterTypeAttachment = std::make_unique<ComboBoxAttachment> (apvts, LautaParams::filterType.getParamID(), filterTypeBox);

    // 3. SATURATION
    addAndMakeVisible (satEnableBtn);
    satEnableAttachment = std::make_unique<ButtonAttachment> (apvts, LautaParams::satEnabled.getParamID(), satEnableBtn);

    setupRotary (satDriveSlider, satDriveLabel, "Drive", juce::Colour (0xfff59e0b));
    satDriveSlider.setTextValueSuffix (" dB");
    satDriveAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::satDrive.getParamID(), satDriveSlider);

    setupRotary (satMixSlider, satMixLabel, "Mix", juce::Colour (0xfff59e0b));
    satMixAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::satMix.getParamID(), satMixSlider);

    // 4. DELAY
    addAndMakeVisible (delayEnableBtn);
    delayEnableAttachment = std::make_unique<ButtonAttachment> (apvts, LautaParams::delayEnabled.getParamID(), delayEnableBtn);

    delayDivisionBox.addItemList ({ "1/16", "1/8", "1/8 D", "1/4", "1/2" }, 1);
    addAndMakeVisible (delayDivisionBox);
    delayDivisionAttachment = std::make_unique<ComboBoxAttachment> (apvts, LautaParams::delayDivision.getParamID(), delayDivisionBox);

    setupRotary (delayFeedbackSlider, delayFeedbackLabel, "Fdbk", juce::Colour (0xff38bdf8));
    delayFeedbackAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::delayFeedback.getParamID(), delayFeedbackSlider);

    setupRotary (delayMixSlider, delayMixLabel, "Mix", juce::Colour (0xff38bdf8));
    delayMixAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::delayMix.getParamID(), delayMixSlider);

    // 5. REVERB
    addAndMakeVisible (reverbEnableBtn);
    reverbEnableAttachment = std::make_unique<ButtonAttachment> (apvts, LautaParams::reverbEnabled.getParamID(), reverbEnableBtn);

    setupRotary (reverbSizeSlider, reverbSizeLabel, "Size", juce::Colour (0xff818cf8));
    reverbSizeAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::reverbSize.getParamID(), reverbSizeSlider);

    setupRotary (reverbDampingSlider, reverbDampingLabel, "Damp", juce::Colour (0xff818cf8));
    reverbDampingAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::reverbDamping.getParamID(), reverbDampingSlider);

    setupRotary (reverbMixSlider, reverbMixLabel, "Mix", juce::Colour (0xff818cf8));
    reverbMixAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::reverbMix.getParamID(), reverbMixSlider);

    // 6. REPEAT
    addAndMakeVisible (repeatEnableBtn);
    repeatEnableAttachment = std::make_unique<ButtonAttachment> (apvts, LautaParams::repeatEnabled.getParamID(), repeatEnableBtn);

    repeatDivisionBox.addItemList ({ "1/4", "1/8", "1/16", "1/32" }, 1);
    addAndMakeVisible (repeatDivisionBox);
    repeatDivisionAttachment = std::make_unique<ComboBoxAttachment> (apvts, LautaParams::repeatDivision.getParamID(), repeatDivisionBox);

    setupRotary (repeatMixSlider, repeatMixLabel, "Mix", juce::Colour (0xffe879f9));
    repeatMixAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::repeatMix.getParamID(), repeatMixSlider);

    // 7. MASTER CONTROLS
    setupRotary (masterMixSlider, masterMixLabel, "MASTER MIX", juce::Colour (0xfff8fafc));
    masterMixAttachment = std::make_unique<SliderAttachment> (apvts, LautaParams::masterMix.getParamID(), masterMixSlider);

    addAndMakeVisible (bypassButton);
    bypassAttachment = std::make_unique<ButtonAttachment> (apvts, LautaParams::masterBypass.getParamID(), bypassButton);

    addAndMakeVisible (resetMacrosButton);
    resetMacrosButton.onClick = [this]
    {
        auto& tree = audioProcessor.getAPVTS();
        if (auto* pE = tree.getParameter (LautaParams::macroEnergy.getParamID()))   pE->setValueNotifyingHost (0.0f);
        if (auto* pS = tree.getParameter (LautaParams::macroSpace.getParamID()))    pS->setValueNotifyingHost (0.0f);
        if (auto* pM = tree.getParameter (LautaParams::macroMovement.getParamID())) pM->setValueNotifyingHost (0.0f);
        if (auto* pW = tree.getParameter (LautaParams::macroWild.getParamID()))     pW->setValueNotifyingHost (0.0f);
    };

    presetBox.addItemList ({ "PRESET: INIT", "PRESET: GROOVE", "PRESET: BUILD", "PRESET: BREAKDOWN" }, 1);
    presetBox.setSelectedId (1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId() - 1;
        audioProcessor.setCurrentProgram (id);
    };
    addAndMakeVisible (presetBox);

    // 8. STATUS BAR LABELS
    auto setupStatusLabel = [this] (juce::Label& lbl, const juce::String& initialText)
    {
        lbl.setText (initialText, juce::dontSendNotification);
        lbl.setFont (12.0f);
        lbl.setColour (juce::Label::textColourId, juce::Colour (0xff94a3b8));
        lbl.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (lbl);
    };

    setupStatusLabel (bpmLabel, "120.0 BPM");
    setupStatusLabel (timeSigLabel, "4 / 4");
    setupStatusLabel (syncStatusLabel, "HOST SYNC");
    setupStatusLabel (sampleRateLabel, "44.1 kHz");

    setSize (940, 580);
    startTimerHz (25); // 25 Hz UI refresh for status bar
}

LautaFXAudioProcessorEditor::~LautaFXAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void LautaFXAudioProcessorEditor::setupRotary (juce::Slider& slider, juce::Label& label,
                                               const juce::String& text, juce::Colour fillColour,
                                               bool isMacro)
{
    slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, isMacro ? 64 : 54, 18);
    slider.setColour (juce::Slider::rotarySliderFillColourId, fillColour);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffcbd5e1));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    if (isMacro)
        slider.getProperties().set ("isMacro", true);

    addAndMakeVisible (slider);

    label.setText (text, juce::dontSendNotification);
    label.setFont (isMacro ? 13.0f : 11.0f);
    label.setColour (juce::Label::textColourId, isMacro ? fillColour : juce::Colour (0xff94a3b8));
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void LautaFXAudioProcessorEditor::timerCallback()
{
    const auto info = audioProcessor.getTransportInfo();

    bpmLabel.setText (juce::String (info.bpm, 1) + " BPM", juce::dontSendNotification);
    timeSigLabel.setText (juce::String (info.timeSigNumerator) + " / " + juce::String (info.timeSigDenominator),
                          juce::dontSendNotification);

    if (info.isPlaying)
    {
        syncStatusLabel.setText ("● PLAYING", juce::dontSendNotification);
        syncStatusLabel.setColour (juce::Label::textColourId, juce::Colour (0xff2dd4bf));
    }
    else
    {
        syncStatusLabel.setText ("■ READY", juce::dontSendNotification);
        syncStatusLabel.setColour (juce::Label::textColourId, juce::Colour (0xff94a3b8));
    }

    sampleRateLabel.setText (juce::String (info.sampleRate / 1000.0, 1) + " kHz",
                             juce::dontSendNotification);
}

void LautaFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff101116));

    // Top Header Background
    auto headerBounds = juce::Rectangle<int> (0, 0, getWidth(), 64);
    g.setColour (juce::Colour (0xff151720));
    g.fillRect (headerBounds);

    g.setColour (juce::Colour (0xff252834));
    g.drawHorizontalLine (64, 0.0f, static_cast<float> (getWidth()));

    // Title
    g.setColour (juce::Colour (0xfff8fafc));
    g.setFont (juce::Font (22.0f).boldened());
    g.drawText ("LAUTA FX", 24, 12, 140, 24, juce::Justification::centredLeft);

    g.setFont (9.5f);
    g.setColour (juce::Colour (0xff64748b));
    g.drawText ("MASTER PERFORMANCE PROCESSOR", 25, 36, 220, 16, juce::Justification::centredLeft);

    // Status pill container in header
    auto statusPill = juce::Rectangle<int> (270, 16, 260, 32).toFloat();
    g.setColour (juce::Colour (0xff1c1f2a));
    g.fillRoundedRectangle (statusPill, 6.0f);
    g.setColour (juce::Colour (0xff2c303f));
    g.drawRoundedRectangle (statusPill, 6.0f, 1.0f);

    // Divider line between Macros and Modules
    g.setColour (juce::Colour (0xff212430));
    g.drawHorizontalLine (335, 20.0f, static_cast<float> (getWidth() - 20));

    // Module section card backgrounds
    const int moduleY = 345;
    const int moduleH = 215;
    const int cardW = 172;
    const int gap = 12;

    for (int i = 0; i < 5; ++i)
    {
        auto cardBounds = juce::Rectangle<int> (20 + i * (cardW + gap), moduleY, cardW, moduleH).toFloat();
        g.setColour (juce::Colour (0xff161822));
        g.fillRoundedRectangle (cardBounds, 8.0f);

        g.setColour (juce::Colour (0xff262a38));
        g.drawRoundedRectangle (cardBounds, 8.0f, 1.0f);
    }
}

void LautaFXAudioProcessorEditor::resized()
{
    // Status Bar in Header
    const int statusY = 22;
    bpmLabel.setBounds (275, statusY, 65, 20);
    timeSigLabel.setBounds (345, statusY, 45, 20);
    syncStatusLabel.setBounds (395, statusY, 65, 20);
    sampleRateLabel.setBounds (465, statusY, 60, 20);

    // Header Controls (Right side)
    presetBox.setBounds (getWidth() - 380, 18, 160, 28);
    resetMacrosButton.setBounds (getWidth() - 210, 18, 110, 28);
    bypassButton.setBounds (getWidth() - 90, 18, 70, 28);

    // Center Macros Section
    const int macroDialSize = 130;
    const int macroY = 100;
    const int macroSpacing = 165;
    const int macroStartX = 40;

    macroEnergyLabel.setBounds (macroStartX, macroY - 24, macroDialSize, 20);
    macroEnergySlider.setBounds (macroStartX, macroY, macroDialSize, macroDialSize + 24);

    macroSpaceLabel.setBounds (macroStartX + macroSpacing, macroY - 24, macroDialSize, 20);
    macroSpaceSlider.setBounds (macroStartX + macroSpacing, macroY, macroDialSize, macroDialSize + 24);

    macroMovementLabel.setBounds (macroStartX + macroSpacing * 2, macroY - 24, macroDialSize, 20);
    macroMovementSlider.setBounds (macroStartX + macroSpacing * 2, macroY, macroDialSize, macroDialSize + 24);

    macroWildLabel.setBounds (macroStartX + macroSpacing * 3, macroY - 24, macroDialSize, 20);
    macroWildSlider.setBounds (macroStartX + macroSpacing * 3, macroY, macroDialSize, macroDialSize + 24);

    // Master Mix Slider (Right side of Macros)
    const int masterX = getWidth() - 170;
    masterMixLabel.setBounds (masterX, macroY - 24, 130, 20);
    masterMixSlider.setBounds (masterX, macroY + 10, 130, 130);

    // Bottom 5 Module Cards
    const int moduleY = 345;
    const int cardW = 172;
    const int gap = 12;
    const int startX = 20;

    // Card 1: FILTER
    {
        int x = startX;
        filterEnableBtn.setBounds (x + 12, moduleY + 10, 148, 24);
        filterTypeBox.setBounds (x + 12, moduleY + 40, 148, 24);

        filterCutoffLabel.setBounds (x + 8, moduleY + 70, 72, 16);
        filterCutoffSlider.setBounds (x + 8, moduleY + 88, 72, 110);

        filterResLabel.setBounds (x + 92, moduleY + 70, 72, 16);
        filterResSlider.setBounds (x + 92, moduleY + 88, 72, 110);
    }

    // Card 2: SATURATION
    {
        int x = startX + (cardW + gap);
        satEnableBtn.setBounds (x + 12, moduleY + 10, 148, 24);

        satDriveLabel.setBounds (x + 8, moduleY + 70, 72, 16);
        satDriveSlider.setBounds (x + 8, moduleY + 88, 72, 110);

        satMixLabel.setBounds (x + 92, moduleY + 70, 72, 16);
        satMixSlider.setBounds (x + 92, moduleY + 88, 72, 110);
    }

    // Card 3: DELAY
    {
        int x = startX + (cardW + gap) * 2;
        delayEnableBtn.setBounds (x + 12, moduleY + 10, 148, 24);
        delayDivisionBox.setBounds (x + 12, moduleY + 40, 148, 24);

        delayFeedbackLabel.setBounds (x + 8, moduleY + 70, 72, 16);
        delayFeedbackSlider.setBounds (x + 8, moduleY + 88, 72, 110);

        delayMixLabel.setBounds (x + 92, moduleY + 70, 72, 16);
        delayMixSlider.setBounds (x + 92, moduleY + 88, 72, 110);
    }

    // Card 4: REVERB
    {
        int x = startX + (cardW + gap) * 3;
        reverbEnableBtn.setBounds (x + 12, moduleY + 10, 148, 24);

        const int subW = 48;
        reverbSizeLabel.setBounds (x + 8, moduleY + 70, subW, 16);
        reverbSizeSlider.setBounds (x + 8, moduleY + 88, subW, 110);

        reverbDampingLabel.setBounds (x + 62, moduleY + 70, subW, 16);
        reverbDampingSlider.setBounds (x + 62, moduleY + 88, subW, 110);

        reverbMixLabel.setBounds (x + 116, moduleY + 70, subW, 16);
        reverbMixSlider.setBounds (x + 116, moduleY + 88, subW, 110);
    }

    // Card 5: REPEAT
    {
        int x = startX + (cardW + gap) * 4;
        repeatEnableBtn.setBounds (x + 12, moduleY + 10, 148, 24);
        repeatDivisionBox.setBounds (x + 12, moduleY + 40, 148, 24);

        repeatMixLabel.setBounds (x + 46, moduleY + 70, 80, 16);
        repeatMixSlider.setBounds (x + 46, moduleY + 88, 80, 110);
    }
}
