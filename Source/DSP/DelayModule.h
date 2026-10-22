#pragma once

#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <vector>

namespace LautaDSP
{

class DelayModule
{
public:
    enum Division
    {
        Div_1_16 = 0,
        Div_1_8,
        Div_1_8D,
        Div_1_4,
        Div_1_2,
        NumDivisions
    };

    DelayModule() = default;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate > 0.0 ? spec.sampleRate : 44100.0;

        // Maximum delay time: 4 seconds
        maxDelaySamples = static_cast<int> (sampleRate * 4.0);
        delayBufferL.assign (static_cast<size_t> (maxDelaySamples), 0.0f);
        delayBufferR.assign (static_cast<size_t> (maxDelaySamples), 0.0f);
        writeIndex = 0;

        feedbackSmoothed.reset (sampleRate, 0.03);
        mixSmoothed.reset (sampleRate, 0.02);
        delayTimeSmoothedL.reset (sampleRate, 0.05);
        delayTimeSmoothedR.reset (sampleRate, 0.05);
        dampingSmoothed.reset (sampleRate, 0.02);

        feedbackSmoothed.setCurrentAndTargetValue (0.3f);
        mixSmoothed.setCurrentAndTargetValue (0.0f);
        dampingSmoothed.setCurrentAndTargetValue (8000.0f);

        updateDelayTimes();

        filterL.prepare (spec);
        filterR.prepare (spec);
        filterL.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        filterR.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        filterL.reset();
        filterR.reset();
    }

    void reset()
    {
        std::fill (delayBufferL.begin(), delayBufferL.end(), 0.0f);
        std::fill (delayBufferR.begin(), delayBufferR.end(), 0.0f);
        writeIndex = 0;
        filterL.reset();
        filterR.reset();
    }

    void setTempo (double bpm)
    {
        currentBpm = (bpm >= 20.0 && bpm <= 400.0) ? bpm : 120.0;
        updateDelayTimes();
    }

    void setDivision (Division div)
    {
        currentDivision = div;
        updateDelayTimes();
    }

    void setFeedback (float fb)
    {
        // Safe cap at 0.95 to prevent unbounded buildup
        feedbackSmoothed.setTargetValue (juce::jlimit (0.0f, 0.95f, fb));
    }

    void setDampingFrequency (float cutoffHz)
    {
        const float maxCutoff = static_cast<float> (sampleRate * 0.49);
        dampingSmoothed.setTargetValue (juce::jlimit (500.0f, maxCutoff, cutoffHz));
    }

    void setMix (float mixFraction)
    {
        mixSmoothed.setTargetValue (juce::jlimit (0.0f, 1.0f, mixFraction));
    }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int numSamples = buffer.getNumSamples();
        if (numSamples == 0 || buffer.getNumChannels() < 2 || maxDelaySamples <= 0)
            return;

        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);

        for (int i = 0; i < numSamples; ++i)
        {
            const float mix = mixSmoothed.getNextValue();
            const float fb  = feedbackSmoothed.getNextValue();
            const float dampFreq = dampingSmoothed.getNextValue();
            const float delaySamplesL = delayTimeSmoothedL.getNextValue();
            const float delaySamplesR = delayTimeSmoothedR.getNextValue();

            filterL.setCutoffFrequency (dampFreq);
            filterR.setCutoffFrequency (dampFreq);

            // Read delayed samples with linear interpolation
            const float delayedL = readDelaySample (delayBufferL, delaySamplesL);
            const float delayedR = readDelaySample (delayBufferR, delaySamplesR);

            // Feedback loop processing with low-pass damping and soft saturation
            float fbSignalL = filterL.processSample (0, delayedL * fb);
            float fbSignalR = filterR.processSample (0, delayedR * fb);

            // Soft-clip feedback to prevent runaway
            fbSignalL = std::tanh (fbSignalL);
            fbSignalR = std::tanh (fbSignalR);

            const float inL = left[i];
            const float inR = right[i];

            // Write input + feedback into delay line
            delayBufferL[static_cast<size_t> (writeIndex)] = inL + fbSignalL;
            delayBufferR[static_cast<size_t> (writeIndex)] = inR + fbSignalR;

            // Output mix
            left[i]  = inL + mix * delayedL;
            right[i] = inR + mix * delayedR;

            if (++writeIndex >= maxDelaySamples)
                writeIndex = 0;
        }
    }

private:
    void updateDelayTimes()
    {
        const double quarterSec = 60.0 / currentBpm;
        double mult = 0.5; // default 1/8

        switch (currentDivision)
        {
            case Div_1_16:       mult = 0.25; break;
            case Div_1_8:        mult = 0.5;  break;
            case Div_1_8D:       mult = 0.75; break;
            case Div_1_4:        mult = 1.0;  break;
            case Div_1_2:        mult = 2.0;  break;
            case NumDivisions:
            default:             mult = 0.5;  break;
        }

        const float targetSamplesL = static_cast<float> (quarterSec * mult * sampleRate);
        // Subtle stereo offset (2% faster on right channel for wide stereo ping-pong feel)
        const float targetSamplesR = targetSamplesL * 1.02f;

        const float safeMax = static_cast<float> (maxDelaySamples - 4);
        delayTimeSmoothedL.setTargetValue (juce::jlimit (10.0f, safeMax, targetSamplesL));
        delayTimeSmoothedR.setTargetValue (juce::jlimit (10.0f, safeMax, targetSamplesR));
    }

    float readDelaySample (const std::vector<float>& buf, float delayInSamples) const noexcept
    {
        float readPos = static_cast<float> (writeIndex) - delayInSamples;
        while (readPos < 0.0f)
            readPos += static_cast<float> (maxDelaySamples);

        const int iPos0 = static_cast<int> (readPos) % maxDelaySamples;
        const int iPos1 = (iPos0 + 1) % maxDelaySamples;
        const float frac = readPos - std::floor (readPos);

        return buf[static_cast<size_t> (iPos0)] * (1.0f - frac) + buf[static_cast<size_t> (iPos1)] * frac;
    }

    double sampleRate = 44100.0;
    double currentBpm = 120.0;
    Division currentDivision = Div_1_8;

    int maxDelaySamples = 0;
    int writeIndex = 0;
    std::vector<float> delayBufferL;
    std::vector<float> delayBufferR;

    juce::dsp::StateVariableTPTFilter<float> filterL;
    juce::dsp::StateVariableTPTFilter<float> filterR;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> feedbackSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayTimeSmoothedL;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> delayTimeSmoothedR;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dampingSmoothed;
};

} // namespace LautaDSP
