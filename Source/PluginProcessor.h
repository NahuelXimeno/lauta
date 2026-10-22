#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "Common/Parameters.h"
#include "Common/MacroManager.h"
#include "DSP/FilterModule.h"
#include "DSP/SaturationModule.h"
#include "DSP/DelayModule.h"
#include "DSP/ReverbModule.h"
#include "DSP/RepeatModule.h"
#include "DSP/SafetyLimiter.h"

class LautaFXAudioProcessor : public juce::AudioProcessor
{
public:
    LautaFXAudioProcessor();
    ~LautaFXAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }

    // Transport info for UI
    struct TransportInfo
    {
        double bpm = 120.0;
        int timeSigNumerator = 4;
        int timeSigDenominator = 4;
        bool isPlaying = false;
        double sampleRate = 44100.0;
    };

    TransportInfo getTransportInfo() const noexcept;

    void loadPreset (int presetIndex);

private:
    juce::AudioProcessorValueTreeState apvts;

    // DSP Modules
    LautaDSP::FilterModule     filterModule;
    LautaDSP::SaturationModule saturationModule;
    LautaDSP::DelayModule      delayModule;
    LautaDSP::ReverbModule     reverbModule;
    LautaDSP::RepeatModule     repeatModule;
    LautaDSP::SafetyLimiter    safetyLimiter;

    // Dry buffer for master mix crossfade (pre-allocated)
    juce::AudioBuffer<float> dryBuffer;

    // Atomic cached parameter pointers for lock-free audio thread reads
    std::atomic<float>* pMacroEnergy   = nullptr;
    std::atomic<float>* pMacroSpace    = nullptr;
    std::atomic<float>* pMacroMovement = nullptr;
    std::atomic<float>* pMacroWild     = nullptr;

    std::atomic<float>* pFilterEnabled   = nullptr;
    std::atomic<float>* pFilterCutoff    = nullptr;
    std::atomic<float>* pFilterResonance = nullptr;
    std::atomic<float>* pFilterType      = nullptr;

    std::atomic<float>* pSatEnabled = nullptr;
    std::atomic<float>* pSatDrive   = nullptr;
    std::atomic<float>* pSatMix     = nullptr;

    std::atomic<float>* pDelayEnabled   = nullptr;
    std::atomic<float>* pDelayDivision  = nullptr;
    std::atomic<float>* pDelayFeedback  = nullptr;
    std::atomic<float>* pDelayMix       = nullptr;

    std::atomic<float>* pReverbEnabled = nullptr;
    std::atomic<float>* pReverbSize    = nullptr;
    std::atomic<float>* pReverbDamping = nullptr;
    std::atomic<float>* pReverbMix     = nullptr;

    std::atomic<float>* pRepeatEnabled  = nullptr;
    std::atomic<float>* pRepeatDivision = nullptr;
    std::atomic<float>* pRepeatMix      = nullptr;

    std::atomic<float>* pMasterMix    = nullptr;
    std::atomic<float>* pMasterBypass = nullptr;

    // Transport state
    std::atomic<double> currentBpm { 120.0 };
    std::atomic<int>    timeSigNum { 4 };
    std::atomic<int>    timeSigDen { 4 };
    std::atomic<bool>   hostPlaying { false };
    std::atomic<double> hostSampleRate { 44100.0 };

    int currentProgramIndex = 0;
    const juce::StringArray presetNames { "INIT", "GROOVE", "BUILD", "BREAKDOWN" };

    void cacheParameterPointers();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LautaFXAudioProcessor)
};
