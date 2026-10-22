#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace LautaParams
{
    // Macro IDs
    inline const juce::ParameterID macroEnergy   { "macroEnergy", 1 };
    inline const juce::ParameterID macroSpace    { "macroSpace", 1 };
    inline const juce::ParameterID macroMovement { "macroMovement", 1 };
    inline const juce::ParameterID macroWild     { "macroWild", 1 };

    // Filter IDs
    inline const juce::ParameterID filterEnabled   { "filterEnabled", 1 };
    inline const juce::ParameterID filterCutoff    { "filterCutoff", 1 };
    inline const juce::ParameterID filterResonance { "filterResonance", 1 };
    inline const juce::ParameterID filterType      { "filterType", 1 };

    // Saturation IDs
    inline const juce::ParameterID satEnabled { "satEnabled", 1 };
    inline const juce::ParameterID satDrive   { "satDrive", 1 };
    inline const juce::ParameterID satMix     { "satMix", 1 };

    // Delay IDs
    inline const juce::ParameterID delayEnabled   { "delayEnabled", 1 };
    inline const juce::ParameterID delayDivision  { "delayDivision", 1 };
    inline const juce::ParameterID delayFeedback  { "delayFeedback", 1 };
    inline const juce::ParameterID delayMix       { "delayMix", 1 };

    // Reverb IDs
    inline const juce::ParameterID reverbEnabled { "reverbEnabled", 1 };
    inline const juce::ParameterID reverbSize    { "reverbSize", 1 };
    inline const juce::ParameterID reverbDamping { "reverbDamping", 1 };
    inline const juce::ParameterID reverbMix     { "reverbMix", 1 };

    // Repeat IDs
    inline const juce::ParameterID repeatEnabled  { "repeatEnabled", 1 };
    inline const juce::ParameterID repeatDivision { "repeatDivision", 1 };
    inline const juce::ParameterID repeatMix      { "repeatMix", 1 };

    // Master IDs
    inline const juce::ParameterID masterMix    { "masterMix", 1 };
    inline const juce::ParameterID masterBypass { "masterBypass", 1 };

    inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        // 1. MACROS (Top-level automatable parameters)
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            macroEnergy, "ENERGY", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            macroSpace, "SPACE", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            macroMovement, "MOVEMENT", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            macroWild, "WILD", juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

        // 2. FILTER MODULE
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            filterEnabled, "Filter Enable", true));

        // Skewed logarithmic range for musical filter sweep
        juce::NormalisableRange<float> cutoffRange (20.0f, 20000.0f, 0.1f, 0.25f);
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            filterCutoff, "Filter Cutoff", cutoffRange, 20000.0f,
            juce::AudioParameterFloatAttributes().withLabel ("Hz")));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            filterResonance, "Filter Resonance", juce::NormalisableRange<float> (0.1f, 5.0f, 0.01f), 0.707f));

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            filterType, "Filter Type", juce::StringArray { "LowPass", "HighPass", "BandPass" }, 0));

        // 3. SATURATION MODULE
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            satEnabled, "Sat Enable", true));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            satDrive, "Sat Drive", juce::NormalisableRange<float> (0.0f, 24.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("dB")));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            satMix, "Sat Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

        // 4. DELAY MODULE
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            delayEnabled, "Delay Enable", true));

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            delayDivision, "Delay Division", juce::StringArray { "1/16", "1/8", "1/8 D", "1/4", "1/2" }, 2));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            delayFeedback, "Delay Feedback", juce::NormalisableRange<float> (0.0f, 0.95f, 0.01f), 0.35f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            delayMix, "Delay Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

        // 5. REVERB MODULE
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            reverbEnabled, "Reverb Enable", true));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            reverbSize, "Reverb Size", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.6f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            reverbDamping, "Reverb Damping", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.4f));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            reverbMix, "Reverb Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

        // 6. REPEAT / GLITCH MODULE
        params.push_back (std::make_unique<juce::AudioParameterBool> (
            repeatEnabled, "Repeat Enable", true));

        params.push_back (std::make_unique<juce::AudioParameterChoice> (
            repeatDivision, "Repeat Division", juce::StringArray { "1/4", "1/8", "1/16", "1/32" }, 2));

        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            repeatMix, "Repeat Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.0f));

        // 7. MASTER CONTROLS
        params.push_back (std::make_unique<juce::AudioParameterFloat> (
            masterMix, "Master Mix", juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 1.0f));

        params.push_back (std::make_unique<juce::AudioParameterBool> (
            masterBypass, "Master Bypass", false));

        return { params.begin(), params.end() };
    }
} // namespace LautaParams
