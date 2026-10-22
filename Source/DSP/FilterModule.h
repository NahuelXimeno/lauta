#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace LautaDSP
{

class FilterModule
{
public:
    enum class FilterType
    {
        LowPass = 0,
        HighPass,
        BandPass
    };

    FilterModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;

        leftFilter.prepare (spec);
        rightFilter.prepare (spec);

        leftFilter.reset();
        rightFilter.reset();

        cutoffSmoothed.reset (spec.sampleRate, 0.02); // 20ms smoothing
        resonanceSmoothed.reset (spec.sampleRate, 0.02);

        cutoffSmoothed.setCurrentAndTargetValue (1000.0f);
        resonanceSmoothed.setCurrentAndTargetValue (0.707f);

        updateFilterType (currentType);
    }

    void reset()
    {
        leftFilter.reset();
        rightFilter.reset();
    }

    void setType (FilterType type)
    {
        currentType = type;
        updateFilterType (currentType);
    }

    void setCutoff (float targetHz)
    {
        // Clamp to safe nyquist range
        const float maxCutoff = static_cast<float> (sampleRate * 0.49);
        const float clamped = juce::jlimit (20.0f, maxCutoff, targetHz);
        cutoffSmoothed.setTargetValue (clamped);
    }

    void setResonance (float targetRes)
    {
        // Safe resonance range 0.1 to 5.0
        const float clamped = juce::jlimit (0.1f, 5.0f, targetRes);
        resonanceSmoothed.setTargetValue (clamped);
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
            const float currentCutoff = cutoffSmoothed.getNextValue();
            const float currentRes    = resonanceSmoothed.getNextValue();

            leftFilter.setCutoffFrequency (currentCutoff);
            leftFilter.setResonance (currentRes);
            rightFilter.setCutoffFrequency (currentCutoff);
            rightFilter.setResonance (currentRes);

            left[i]  = leftFilter.processSample (0, left[i]);
            right[i] = rightFilter.processSample (0, right[i]);
        }
    }

private:
    void updateFilterType (FilterType type)
    {
        juce::dsp::StateVariableTPTFilterType juceType;
        switch (type)
        {
            case FilterType::LowPass:
                juceType = juce::dsp::StateVariableTPTFilterType::lowpass;
                break;
            case FilterType::HighPass:
                juceType = juce::dsp::StateVariableTPTFilterType::highpass;
                break;
            case FilterType::BandPass:
                juceType = juce::dsp::StateVariableTPTFilterType::bandpass;
                break;
        }

        leftFilter.setType (juceType);
        rightFilter.setType (juceType);
    }

    double sampleRate = 44100.0;
    FilterType currentType = FilterType::LowPass;

    juce::dsp::StateVariableTPTFilter<float> leftFilter;
    juce::dsp::StateVariableTPTFilter<float> rightFilter;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> cutoffSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> resonanceSmoothed;
};

} // namespace LautaDSP
