#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include <cmath>

namespace LautaDSP
{

class RepeatModule
{
public:
    enum Division
    {
        Div_1_4 = 0,
        Div_1_8,
        Div_1_16,
        Div_1_32,
        NumDivisions
    };

    RepeatModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;

        // Maximum slice buffer: 2 seconds stereo
        maxSliceSamples = static_cast<int> (sampleRate * 2.0);
        sliceBufferL.assign (static_cast<size_t> (maxSliceSamples), 0.0f);
        sliceBufferR.assign (static_cast<size_t> (maxSliceSamples), 0.0f);

        recordBufferL.assign (static_cast<size_t> (maxSliceSamples), 0.0f);
        recordBufferR.assign (static_cast<size_t> (maxSliceSamples), 0.0f);

        recordIndex = 0;
        playIndex   = 0;
        sliceLength = static_cast<int> (sampleRate * 0.25); // default 1/8 note at 120bpm

        mixSmoothed.reset (sampleRate, 0.015);
        mixSmoothed.setCurrentAndTargetValue (0.0f);

        decaySmoothed.reset (sampleRate, 0.02);
        decaySmoothed.setCurrentAndTargetValue (0.0f);

        isRepeating = false;
        updateSliceLength();
    }

    void reset()
    {
        std::fill (sliceBufferL.begin(), sliceBufferL.end(), 0.0f);
        std::fill (sliceBufferR.begin(), sliceBufferR.end(), 0.0f);
        std::fill (recordBufferL.begin(), recordBufferL.end(), 0.0f);
        std::fill (recordBufferR.begin(), recordBufferR.end(), 0.0f);
        recordIndex = 0;
        playIndex   = 0;
        isRepeating = false;
    }

    void setTempo (double bpm)
    {
        currentBpm = (bpm >= 20.0 && bpm <= 400.0) ? bpm : 120.0;
        updateSliceLength();
    }

    void setDivision (Division div)
    {
        currentDivision = div;
        updateSliceLength();
    }

    void setDecay (float decayRate)
    {
        decaySmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, decayRate));
    }

    void setMix (float mixFraction)
    {
        const float clamped = juce::jlimit (0.0f, 1.0f, mixFraction);
        mixSmoothed.setTargetValue (clamped);

        // When mix rises above threshold, engage repeat mode and freeze the buffer
        if (clamped > 0.05f && ! isRepeating)
        {
            freezeCurrentSlice();
            isRepeating = true;
        }
        else if (clamped <= 0.001f && isRepeating)
        {
            isRepeating = false;
        }
    }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0 || buffer.getNumChannels() < 2 || maxSliceSamples <= 0)
            return;

        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);

        const int fadeSamples = juce::jlimit (16, 256, static_cast<int> (sampleRate * 0.005)); // 5ms fade

        for (int i = 0; i < numSamples; ++i)
        {
            const float mix = mixSmoothed.getNextValue();
            const float decay = decaySmoothed.getNextValue();

            const float inL = left[i];
            const float inR = right[i];

            // Always maintain a rolling record buffer of recent audio
            recordBufferL[static_cast<size_t> (recordIndex)] = inL;
            recordBufferR[static_cast<size_t> (recordIndex)] = inR;

            if (++recordIndex >= maxSliceSamples)
                recordIndex = 0;

            if (isRepeating && sliceLength > 0)
            {
                // Read from frozen slice buffer
                float repL = sliceBufferL[static_cast<size_t> (playIndex)];
                float repR = sliceBufferR[static_cast<size_t> (playIndex)];

                // Apply crossfade envelope near loop boundaries to prevent clicks
                if (playIndex < fadeSamples)
                {
                    float gain = static_cast<float> (playIndex) / static_cast<float> (fadeSamples);
                    repL *= gain;
                    repR *= gain;
                }
                else if (playIndex > sliceLength - fadeSamples)
                {
                    float gain = static_cast<float> (sliceLength - playIndex) / static_cast<float> (fadeSamples);
                    repL *= gain;
                    repR *= gain;
                }

                // Progressive decay per loop if enabled
                if (decay > 0.01f)
                {
                    const float cycleFactor = 1.0f - (decay * 0.5f * (static_cast<float> (playIndex) / static_cast<float> (sliceLength)));
                    repL *= cycleFactor;
                    repR *= cycleFactor;
                }

                left[i]  = inL + mix * (repL - inL);
                right[i] = inR + mix * (repR - inR);

                if (++playIndex >= sliceLength)
                    playIndex = 0;
            }
        }
    }

private:
    void updateSliceLength()
    {
        const double quarterSec = 60.0 / currentBpm;
        double mult = 0.5; // 1/8

        switch (currentDivision)
        {
            case Div_1_4:        mult = 1.0;   break;
            case Div_1_8:        mult = 0.5;   break;
            case Div_1_16:       mult = 0.25;  break;
            case Div_1_32:       mult = 0.125; break;
            case NumDivisions:
            default:             mult = 0.5;   break;
        }

        const int targetSamples = static_cast<int> (quarterSec * mult * sampleRate);
        sliceLength = juce::jlimit (64, maxSliceSamples - 1, targetSamples);
    }

    void freezeCurrentSlice()
    {
        if (sliceLength <= 0 || maxSliceSamples <= 0)
            return;

        // Copy the most recent `sliceLength` samples from `recordBuffer` into `sliceBuffer`
        int srcIdx = recordIndex - sliceLength;
        while (srcIdx < 0)
            srcIdx += maxSliceSamples;

        for (int i = 0; i < sliceLength; ++i)
        {
            sliceBufferL[static_cast<size_t> (i)] = recordBufferL[static_cast<size_t> (srcIdx)];
            sliceBufferR[static_cast<size_t> (i)] = recordBufferR[static_cast<size_t> (srcIdx)];

            if (++srcIdx >= maxSliceSamples)
                srcIdx = 0;
        }

        playIndex = 0;
    }

    double sampleRate = 44100.0;
    double currentBpm = 120.0;
    Division currentDivision = Div_1_16;

    int maxSliceSamples = 0;
    int sliceLength = 0;
    int recordIndex = 0;
    int playIndex   = 0;
    bool isRepeating = false;

    std::vector<float> sliceBufferL;
    std::vector<float> sliceBufferR;
    std::vector<float> recordBufferL;
    std::vector<float> recordBufferR;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> decaySmoothed;
};

} // namespace LautaDSP
