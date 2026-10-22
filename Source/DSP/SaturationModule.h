#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace LautaDSP
{

class SaturationModule
{
public:
    SaturationModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;
        driveSmoothed.reset (sampleRate, 0.02);
        mixSmoothed.reset (sampleRate, 0.02);

        driveSmoothed.setCurrentAndTargetValue (1.0f);
        mixSmoothed.setCurrentAndTargetValue (0.0f);
    }

    void reset()
    {
    }

    void setDrive (float driveDb)
    {
        // Drive in dB: 0 to 24 dB
        const float linearGain = juce::Decibels::decibelsToGain (juce::jlimit (0.0f, 24.0f, driveDb));
        driveSmoothed.setTargetValue (linearGain);
    }

    void setMix (float mixFraction)
    {
        mixSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, mixFraction));
    }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0 || buffer.getNumChannels() < 2)
            return;

        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float drive = driveSmoothed.getNextValue();
            const float mix   = mixSmoothed.getNextValue();

            if (mix <= 0.0001f && drive <= 1.001f)
                continue;

            // Output compensation formula so volume doesn't spike as drive increases
            // As drive increases from 1.0 upwards, compensation attenuates accordingly
            const float compensation = 1.0f / std::sqrt (1.0f + 0.5f * (drive - 1.0f));

            const float dryL = left[i];
            const float dryR = right[i];

            // Soft-knee asymmetric/symmetric tanh waveshaping
            const float inL = dryL * drive;
            const float inR = dryR * drive;

            // Tanh soft saturation
            const float satL = std::tanh (inL) * compensation;
            const float satR = std::tanh (inR) * compensation;

            left[i]  = dryL + mix * (satL - dryL);
            right[i] = dryR + mix * (satR - dryR);
        }
    }

private:
    double sampleRate = 44100.0;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
};

} // namespace LautaDSP
