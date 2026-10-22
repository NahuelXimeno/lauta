#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace LautaDSP
{

/**
 * High-performance, zero-latency transparent safety ceiling / limiter.
 * Protects the master channel against runaway feedback, filter resonance spikes,
 * or unexpected gain increases without coloring signals below -0.5 dB.
 */
class SafetyLimiter
{
public:
    SafetyLimiter() = default;

    void prepare (double sampleRate) noexcept
    {
        currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
        envelopeL = 0.0f;
        envelopeR = 0.0f;
        // Release time around 50ms
        releaseFactor = static_cast<float> (std::exp (-1.0 / (0.05 * currentSampleRate)));
    }

    void reset() noexcept
    {
        envelopeL = 0.0f;
        envelopeR = 0.0f;
    }

    void process (juce::AudioBuffer<float>& buffer) noexcept
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples  = buffer.getNumSamples();

        if (numChannels < 2)
            return;

        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getWritePointer (1);

        const float threshold = 0.95f; // ~ -0.45 dBFS
        const float ceiling   = 0.99f; // Hard stop

        for (int i = 0; i < numSamples; ++i)
        {
            float l = left[i];
            float r = right[i];

            // Clean any denormals or non-finite values
            if (! std::isfinite (l)) l = 0.0f;
            if (! std::isfinite (r)) r = 0.0f;

            float absL = std::abs (l);
            float absR = std::abs (r);

            envelopeL = std::max (absL, envelopeL * releaseFactor);
            envelopeR = std::max (absR, envelopeR * releaseFactor);

            float peak = std::max (envelopeL, envelopeR);

            if (peak > threshold)
            {
                // Soft-knee compression above threshold
                float excess = peak - threshold;
                float gainReduction = threshold + (excess / (1.0f + excess));
                float gain = (gainReduction / peak);

                l *= gain;
                r *= gain;
            }

            // Hard soft-clipping ceiling for extreme overshoots
            if (std::abs (l) > ceiling)
                l = std::copysign (ceiling * std::tanh (std::abs (l) / ceiling), l);

            if (std::abs (r) > ceiling)
                r = std::copysign (ceiling * std::tanh (std::abs (r) / ceiling), r);

            left[i]  = l;
            right[i] = r;
        }
    }

private:
    double currentSampleRate = 44100.0;
    float envelopeL = 0.0f;
    float envelopeR = 0.0f;
    float releaseFactor = 0.999f;
};

} // namespace LautaDSP
