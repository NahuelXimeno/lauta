#pragma once

#include <juce_core/juce_core.h>
#include <cmath>

namespace LautaParams
{

/**
 * Calculated effective DSP parameters resulting from combining
 * individual base parameter settings with the 4 performance macros.
 */
struct EffectiveParameters
{
    // Filter
    bool  filterEnabled   = true;
    int   filterType      = 0;
    float filterCutoff    = 20000.0f;
    float filterResonance = 0.707f;

    // Saturation
    bool  satEnabled = true;
    float satDrive   = 0.0f;
    float satMix     = 0.0f;

    // Delay
    bool  delayEnabled  = true;
    int   delayDivision = 2; // 1/8D
    float delayFeedback = 0.35f;
    float delayMix      = 0.0f;

    // Reverb
    bool  reverbEnabled = true;
    float reverbSize    = 0.6f;
    float reverbDamping = 0.4f;
    float reverbMix     = 0.0f;

    // Repeat / Glitch
    bool  repeatEnabled  = true;
    int   repeatDivision = 2; // 1/16
    float repeatMix      = 0.0f;
    float repeatDecay    = 0.0f;

    // Master
    float masterMix    = 1.0f;
    bool  masterBypass = false;
};

class MacroManager
{
public:
    static EffectiveParameters compute (
        // Macros
        float energy,
        float space,
        float movement,
        float wild,
        // Base parameter values
        bool  baseFilterEnabled,
        int   baseFilterType,
        float baseFilterCutoff,
        float baseFilterResonance,
        bool  baseSatEnabled,
        float baseSatDrive,
        float baseSatMix,
        bool  baseDelayEnabled,
        int   baseDelayDivision,
        float baseDelayFeedback,
        float baseDelayMix,
        bool  baseReverbEnabled,
        float baseReverbSize,
        float baseReverbDamping,
        float baseReverbMix,
        bool  baseRepeatEnabled,
        int   baseRepeatDivision,
        float baseRepeatMix,
        float baseMasterMix,
        bool  baseMasterBypass)
    {
        EffectiveParameters eff;

        // Clamp macros
        energy   = juce::jlimit (0.0f, 1.0f, energy);
        space    = juce::jlimit (0.0f, 1.0f, space);
        movement = juce::jlimit (0.0f, 1.0f, movement);
        wild     = juce::jlimit (0.0f, 1.0f, wild);

        eff.masterMix    = juce::jlimit (0.0f, 1.0f, baseMasterMix);
        eff.masterBypass = baseMasterBypass;

        // 1. FILTER & ENERGY
        eff.filterEnabled   = baseFilterEnabled;
        eff.filterType      = baseFilterType;
        eff.filterResonance = baseFilterResonance;

        // ENERGY creates build tension:
        // Musical curve: x^1.5 for natural progression
        const float energyCurve = std::pow (energy, 1.5f);

        if (baseFilterType == 0) // LowPass
        {
            // If base cutoff is wide open (> 15kHz), ENERGY sweeps it down for a breakdown sweep
            // Otherwise it sweeps cutoff dynamically
            if (baseFilterCutoff > 10000.0f)
            {
                // Sweeps from base down to 350 Hz at maximum energy
                eff.filterCutoff = baseFilterCutoff * std::pow (350.0f / baseFilterCutoff, energyCurve);
            }
            else
            {
                eff.filterCutoff = juce::jlimit (20.0f, 20000.0f, baseFilterCutoff * (1.0f - 0.7f * energyCurve));
            }
            // Add slight resonance bite during build
            eff.filterResonance = juce::jlimit (0.1f, 4.0f, baseFilterResonance + 1.2f * energyCurve);
        }
        else if (baseFilterType == 1) // HighPass
        {
            // Sweeps high-pass upwards to thin the bass during build (up to 1200 Hz)
            const float targetHP = 1200.0f;
            eff.filterCutoff = baseFilterCutoff + (targetHP - baseFilterCutoff) * energyCurve;
            eff.filterResonance = juce::jlimit (0.1f, 4.0f, baseFilterResonance + 0.8f * energyCurve);
        }
        else // BandPass
        {
            eff.filterCutoff = baseFilterCutoff;
            eff.filterResonance = juce::jlimit (0.1f, 5.0f, baseFilterResonance + 1.5f * energyCurve);
        }

        // 2. SATURATION (ENERGY & WILD)
        eff.satEnabled = baseSatEnabled;
        // ENERGY adds up to +8 dB drive, WILD adds up to +6 dB
        const float macroSatDrive = (energy * 8.0f) + (wild * 6.0f);
        eff.satDrive = juce::jlimit (0.0f, 24.0f, baseSatDrive + macroSatDrive);

        // Mix increases with macro if base mix is low
        const float macroSatMix = std::max (energy * 0.75f, wild * 0.5f);
        eff.satMix = juce::jlimit (0.0f, 1.0f, std::max (baseSatMix, macroSatMix));

        // 3. SPACE (DELAY & REVERB)
        // Space uses musical quadratic curve
        const float spaceCurve = std::pow (space, 1.4f);

        // Reverb
        eff.reverbEnabled = baseReverbEnabled;
        eff.reverbSize    = juce::jlimit (0.0f, 1.0f, baseReverbSize + (1.0f - baseReverbSize) * 0.4f * spaceCurve);
        eff.reverbDamping = baseReverbDamping;
        // Space smoothly blends reverb wet level up to 0.65
        const float spaceReverbMix = spaceCurve * 0.65f;
        eff.reverbMix = juce::jlimit (0.0f, 1.0f, std::max (baseReverbMix, spaceReverbMix));

        // Delay
        eff.delayEnabled  = baseDelayEnabled;
        eff.delayDivision = baseDelayDivision;
        // Space increases delay mix up to 0.45
        const float spaceDelayMix = spaceCurve * 0.45f;
        eff.delayMix = juce::jlimit (0.0f, 1.0f, std::max (baseDelayMix, spaceDelayMix));

        // 4. MOVEMENT (DELAY FEEDBACK & REPEAT)
        // Movement increases delay feedback up to +0.35 (capped at 0.92)
        eff.delayFeedback = juce::jlimit (0.0f, 0.92f, baseDelayFeedback + movement * 0.35f);

        eff.repeatEnabled = baseRepeatEnabled;
        eff.repeatDivision = baseRepeatDivision;
        eff.repeatDecay = 0.0f;

        // When Movement > 0.35, engage rhythmic repeat progressively
        float movementRepeatMix = 0.0f;
        if (movement > 0.35f)
        {
            // Linear ramp from 0.0 to 0.75 as movement goes from 0.35 to 1.0
            movementRepeatMix = (movement - 0.35f) / 0.65f * 0.75f;
        }

        // WILD modulates repeat division and intensity:
        // When WILD is high, division speeds up to 1/16 or 1/32
        if (wild > 0.6f)
            eff.repeatDivision = 3; // 1/32
        else if (wild > 0.25f)
            eff.repeatDivision = 2; // 1/16

        eff.repeatDecay = wild * 0.6f;

        const float wildRepeatMix = wild * 0.85f;
        eff.repeatMix = juce::jlimit (0.0f, 1.0f, std::max ({ baseRepeatMix, movementRepeatMix, wildRepeatMix }));

        return eff;
    }
};

} // namespace LautaParams
