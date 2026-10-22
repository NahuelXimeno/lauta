#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "PluginProcessor.h"
#include "Common/LautaLookAndFeel.h"

class LautaFXAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    explicit LautaFXAudioProcessorEditor (LautaFXAudioProcessor&);
    ~LautaFXAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    LautaFXAudioProcessor& audioProcessor;
    LautaUI::LautaLookAndFeel lautaLookAndFeel;

    using SliderAttachment   = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment   = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    // Macro Controls
    juce::Slider macroEnergySlider;
    juce::Slider macroSpaceSlider;
    juce::Slider macroMovementSlider;
    juce::Slider macroWildSlider;

    juce::Label macroEnergyLabel;
    juce::Label macroSpaceLabel;
    juce::Label macroMovementLabel;
    juce::Label macroWildLabel;

    std::unique_ptr<SliderAttachment> energyAttachment;
    std::unique_ptr<SliderAttachment> spaceAttachment;
    std::unique_ptr<SliderAttachment> movementAttachment;
    std::unique_ptr<SliderAttachment> wildAttachment;

    // Filter Module
    juce::ToggleButton filterEnableBtn { "FILTER" };
    juce::Slider filterCutoffSlider;
    juce::Slider filterResSlider;
    juce::ComboBox filterTypeBox;
    juce::Label filterCutoffLabel { {}, "Cutoff" };
    juce::Label filterResLabel { {}, "Res" };

    std::unique_ptr<ButtonAttachment> filterEnableAttachment;
    std::unique_ptr<SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<SliderAttachment> filterResAttachment;
    std::unique_ptr<ComboBoxAttachment> filterTypeAttachment;

    // Saturation Module
    juce::ToggleButton satEnableBtn { "SATURATION" };
    juce::Slider satDriveSlider;
    juce::Slider satMixSlider;
    juce::Label satDriveLabel { {}, "Drive" };
    juce::Label satMixLabel { {}, "Mix" };

    std::unique_ptr<ButtonAttachment> satEnableAttachment;
    std::unique_ptr<SliderAttachment> satDriveAttachment;
    std::unique_ptr<SliderAttachment> satMixAttachment;

    // Delay Module
    juce::ToggleButton delayEnableBtn { "DELAY" };
    juce::ComboBox delayDivisionBox;
    juce::Slider delayFeedbackSlider;
    juce::Slider delayMixSlider;
    juce::Label delayFeedbackLabel { {}, "Fdbk" };
    juce::Label delayMixLabel { {}, "Mix" };

    std::unique_ptr<ButtonAttachment> delayEnableAttachment;
    std::unique_ptr<ComboBoxAttachment> delayDivisionAttachment;
    std::unique_ptr<SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<SliderAttachment> delayMixAttachment;

    // Reverb Module
    juce::ToggleButton reverbEnableBtn { "REVERB" };
    juce::Slider reverbSizeSlider;
    juce::Slider reverbDampingSlider;
    juce::Slider reverbMixSlider;
    juce::Label reverbSizeLabel { {}, "Size" };
    juce::Label reverbDampingLabel { {}, "Damp" };
    juce::Label reverbMixLabel { {}, "Mix" };

    std::unique_ptr<ButtonAttachment> reverbEnableAttachment;
    std::unique_ptr<SliderAttachment> reverbSizeAttachment;
    std::unique_ptr<SliderAttachment> reverbDampingAttachment;
    std::unique_ptr<SliderAttachment> reverbMixAttachment;

    // Repeat Module
    juce::ToggleButton repeatEnableBtn { "REPEAT" };
    juce::ComboBox repeatDivisionBox;
    juce::Slider repeatMixSlider;
    juce::Label repeatMixLabel { {}, "Mix" };

    std::unique_ptr<ButtonAttachment> repeatEnableAttachment;
    std::unique_ptr<ComboBoxAttachment> repeatDivisionAttachment;
    std::unique_ptr<SliderAttachment> repeatMixAttachment;

    // Master Section
    juce::Slider masterMixSlider;
    juce::Label masterMixLabel { {}, "MASTER MIX" };
    juce::ToggleButton bypassButton { "BYPASS" };
    juce::TextButton resetMacrosButton { "RESET MACROS" };
    juce::ComboBox presetBox;

    std::unique_ptr<SliderAttachment> masterMixAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    // Status Section
    juce::Label bpmLabel;
    juce::Label timeSigLabel;
    juce::Label syncStatusLabel;
    juce::Label sampleRateLabel;

    void setupRotary (juce::Slider& slider, juce::Label& label, const juce::String& text,
                      juce::Colour fillColour, bool isMacro = false);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LautaFXAudioProcessorEditor)
};
