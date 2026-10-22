#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>

namespace LautaDSP
{

class ReverbModule
{
public:
    ReverbModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;

        reverb.setSampleRate (sampleRate);
        reverb.reset();

        mixSmoothed.reset (sampleRate, 0.02);
        mixSmoothed.setCurrentAndTargetValue (0.0f);

        currentParams.roomSize = 0.6f;
        currentParams.damping  = 0.5f;
        currentParams.wetLevel = 1.0f;
        currentParams.dryLevel = 0.0f;
        currentParams.width    = 1.0f;
        currentParams.freezeMode = 0.0f;

        reverb.setParameters (currentParams);

        tempReverbBuffer.setSize (2, static_cast<int> (spec.maximumBlockSize), false, false, true);
    }

    void reset()
    {
        reverb.reset();
    }

    void setRoomSize (float size)
    {
        currentParams.roomSize = juce::jlimit (0.0f, 1.0f, size);
        reverb.setParameters (currentParams);
    }

    void setDamping (float damping)
    {
        currentParams.damping = juce::jlimit (0.0f, 1.0f, damping);
        reverb.setParameters (currentParams);
    }

    void setWidth (float width)
    {
        currentParams.width = juce::jlimit (0.0f, 1.0f, width);
        reverb.setParameters (currentParams);
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

        // Ensure temporary buffer can hold the block
        if (tempReverbBuffer.getNumSamples() < numSamples)
            tempReverbBuffer.setSize (2, numSamples, false, false, true);

        // Copy input to temporary buffer for reverb processing
        tempReverbBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
        tempReverbBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);

        // Process pure wet in temp buffer
        reverb.processStereo (tempReverbBuffer.getWritePointer (0),
                              tempReverbBuffer.getWritePointer (1),
                              numSamples);

        auto* dryL = buffer.getWritePointer (0);
        auto* dryR = buffer.getWritePointer (1);
        const auto* wetL = tempReverbBuffer.getReadPointer (0);
        const auto* wetR = tempReverbBuffer.getReadPointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float mix = mixSmoothed.getNextValue();
            dryL[i] = dryL[i] + mix * wetL[i];
            dryR[i] = dryR[i] + mix * wetR[i];
        }
    }

private:
    double sampleRate = 44100.0;
    juce::Reverb reverb;
    juce::Reverb::Parameters currentParams;

    juce::AudioBuffer<float> tempReverbBuffer;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
};

} // namespace LautaDSP
