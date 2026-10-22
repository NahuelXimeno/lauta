#include "PluginProcessor.h"
#include "PluginEditor.h"

LautaFXAudioProcessor::LautaFXAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", LautaParams::createParameterLayout())
{
    cacheParameterPointers();
}

LautaFXAudioProcessor::~LautaFXAudioProcessor()
{
}

void LautaFXAudioProcessor::cacheParameterPointers()
{
    pMacroEnergy   = apvts.getRawParameterValue (LautaParams::macroEnergy.getParamID());
    pMacroSpace    = apvts.getRawParameterValue (LautaParams::macroSpace.getParamID());
    pMacroMovement = apvts.getRawParameterValue (LautaParams::macroMovement.getParamID());
    pMacroWild     = apvts.getRawParameterValue (LautaParams::macroWild.getParamID());

    pFilterEnabled   = apvts.getRawParameterValue (LautaParams::filterEnabled.getParamID());
    pFilterCutoff    = apvts.getRawParameterValue (LautaParams::filterCutoff.getParamID());
    pFilterResonance = apvts.getRawParameterValue (LautaParams::filterResonance.getParamID());
    pFilterType      = apvts.getRawParameterValue (LautaParams::filterType.getParamID());

    pSatEnabled = apvts.getRawParameterValue (LautaParams::satEnabled.getParamID());
    pSatDrive   = apvts.getRawParameterValue (LautaParams::satDrive.getParamID());
    pSatMix     = apvts.getRawParameterValue (LautaParams::satMix.getParamID());

    pDelayEnabled   = apvts.getRawParameterValue (LautaParams::delayEnabled.getParamID());
    pDelayDivision  = apvts.getRawParameterValue (LautaParams::delayDivision.getParamID());
    pDelayFeedback  = apvts.getRawParameterValue (LautaParams::delayFeedback.getParamID());
    pDelayMix       = apvts.getRawParameterValue (LautaParams::delayMix.getParamID());

    pReverbEnabled = apvts.getRawParameterValue (LautaParams::reverbEnabled.getParamID());
    pReverbSize    = apvts.getRawParameterValue (LautaParams::reverbSize.getParamID());
    pReverbDamping = apvts.getRawParameterValue (LautaParams::reverbDamping.getParamID());
    pReverbMix     = apvts.getRawParameterValue (LautaParams::reverbMix.getParamID());

    pRepeatEnabled  = apvts.getRawParameterValue (LautaParams::repeatEnabled.getParamID());
    pRepeatDivision = apvts.getRawParameterValue (LautaParams::repeatDivision.getParamID());
    pRepeatMix      = apvts.getRawParameterValue (LautaParams::repeatMix.getParamID());

    pMasterMix    = apvts.getRawParameterValue (LautaParams::masterMix.getParamID());
    pMasterBypass = apvts.getRawParameterValue (LautaParams::masterBypass.getParamID());
}

const juce::String LautaFXAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool LautaFXAudioProcessor::acceptsMidi() const
{
    return true;
}

bool LautaFXAudioProcessor::producesMidi() const
{
    return false;
}

bool LautaFXAudioProcessor::isMidiEffect() const
{
    return false;
}

double LautaFXAudioProcessor::getTailLengthSeconds() const
{
    return 2.5; // Allow reverb and delay tails to ring out naturally
}

int LautaFXAudioProcessor::getNumPrograms()
{
    return presetNames.size();
}

int LautaFXAudioProcessor::getCurrentProgram()
{
    return currentProgramIndex;
}

void LautaFXAudioProcessor::setCurrentProgram (int index)
{
    if (index >= 0 && index < presetNames.size())
    {
        currentProgramIndex = index;
        loadPreset (index);
    }
}

const juce::String LautaFXAudioProcessor::getProgramName (int index)
{
    if (index >= 0 && index < presetNames.size())
        return presetNames[index];

    return {};
}

void LautaFXAudioProcessor::changeProgramName (int, const juce::String&)
{
}

void LautaFXAudioProcessor::loadPreset (int presetIndex)
{
    auto setParam = [this] (const juce::ParameterID& id, float value)
    {
        if (auto* param = apvts.getParameter (id.getParamID()))
        {
            float norm = param->getNormalisableRange().convertTo0to1 (value);
            param->setValueNotifyingHost (norm);
        }
    };

    switch (presetIndex)
    {
        case 0: // INIT
            setParam (LautaParams::macroEnergy, 0.0f);
            setParam (LautaParams::macroSpace, 0.0f);
            setParam (LautaParams::macroMovement, 0.0f);
            setParam (LautaParams::macroWild, 0.0f);
            setParam (LautaParams::filterCutoff, 20000.0f);
            setParam (LautaParams::filterResonance, 0.707f);
            setParam (LautaParams::filterType, 0.0f);
            setParam (LautaParams::satDrive, 0.0f);
            setParam (LautaParams::satMix, 0.0f);
            setParam (LautaParams::delayDivision, 2.0f); // 1/8 D
            setParam (LautaParams::delayFeedback, 0.35f);
            setParam (LautaParams::delayMix, 0.0f);
            setParam (LautaParams::reverbSize, 0.6f);
            setParam (LautaParams::reverbDamping, 0.4f);
            setParam (LautaParams::reverbMix, 0.0f);
            setParam (LautaParams::repeatDivision, 2.0f); // 1/16
            setParam (LautaParams::repeatMix, 0.0f);
            setParam (LautaParams::masterMix, 1.0f);
            setParam (LautaParams::masterBypass, 0.0f);
            break;

        case 1: // GROOVE
            setParam (LautaParams::macroEnergy, 0.15f);
            setParam (LautaParams::macroSpace, 0.20f);
            setParam (LautaParams::macroMovement, 0.10f);
            setParam (LautaParams::macroWild, 0.0f);
            setParam (LautaParams::filterCutoff, 18000.0f);
            setParam (LautaParams::filterResonance, 0.75f);
            setParam (LautaParams::satDrive, 3.5f);
            setParam (LautaParams::satMix, 0.35f);
            setParam (LautaParams::delayDivision, 2.0f);
            setParam (LautaParams::delayFeedback, 0.40f);
            setParam (LautaParams::delayMix, 0.22f);
            setParam (LautaParams::reverbSize, 0.55f);
            setParam (LautaParams::reverbMix, 0.18f);
            break;

        case 2: // BUILD
            setParam (LautaParams::macroEnergy, 0.65f);
            setParam (LautaParams::macroSpace, 0.45f);
            setParam (LautaParams::macroMovement, 0.50f);
            setParam (LautaParams::macroWild, 0.30f);
            setParam (LautaParams::filterCutoff, 8000.0f);
            setParam (LautaParams::filterResonance, 1.8f);
            setParam (LautaParams::satDrive, 7.0f);
            setParam (LautaParams::satMix, 0.60f);
            setParam (LautaParams::delayDivision, 1.0f);
            setParam (LautaParams::delayFeedback, 0.65f);
            setParam (LautaParams::delayMix, 0.35f);
            setParam (LautaParams::reverbSize, 0.75f);
            setParam (LautaParams::reverbMix, 0.35f);
            setParam (LautaParams::repeatDivision, 2.0f);
            setParam (LautaParams::repeatMix, 0.40f);
            break;

        case 3: // BREAKDOWN
            setParam (LautaParams::macroEnergy, 0.20f);
            setParam (LautaParams::macroSpace, 0.80f);
            setParam (LautaParams::macroMovement, 0.25f);
            setParam (LautaParams::macroWild, 0.10f);
            setParam (LautaParams::filterCutoff, 3500.0f);
            setParam (LautaParams::filterResonance, 1.2f);
            setParam (LautaParams::satDrive, 4.0f);
            setParam (LautaParams::satMix, 0.40f);
            setParam (LautaParams::delayDivision, 3.0f);
            setParam (LautaParams::delayFeedback, 0.60f);
            setParam (LautaParams::delayMix, 0.50f);
            setParam (LautaParams::reverbSize, 0.90f);
            setParam (LautaParams::reverbDamping, 0.3f);
            setParam (LautaParams::reverbMix, 0.60f);
            break;
    }
}

void LautaFXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    hostSampleRate.store (sampleRate, std::memory_order_relaxed);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumOutputChannels());

    filterModule.prepare (spec);
    saturationModule.prepare (spec);
    delayModule.prepare (spec);
    reverbModule.prepare (spec);
    repeatModule.prepare (spec);
    safetyLimiter.prepare (sampleRate);

    dryBuffer.setSize (2, samplesPerBlock, false, false, true);

    delayModule.setTempo (currentBpm.load (std::memory_order_relaxed));
    repeatModule.setTempo (currentBpm.load (std::memory_order_relaxed));
}

void LautaFXAudioProcessor::releaseResources()
{
    filterModule.reset();
    saturationModule.reset();
    delayModule.reset();
    reverbModule.reset();
    repeatModule.reset();
    safetyLimiter.reset();
}

bool LautaFXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

LautaFXAudioProcessor::TransportInfo LautaFXAudioProcessor::getTransportInfo() const noexcept
{
    TransportInfo info;
    info.bpm = currentBpm.load (std::memory_order_relaxed);
    info.timeSigNumerator = timeSigNum.load (std::memory_order_relaxed);
    info.timeSigDenominator = timeSigDen.load (std::memory_order_relaxed);
    info.isPlaying = hostPlaying.load (std::memory_order_relaxed);
    info.sampleRate = hostSampleRate.load (std::memory_order_relaxed);
    return info;
}

void LautaFXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples             = buffer.getNumSamples();

    // Clear unused output channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamples);

    if (numSamples == 0 || totalNumInputChannels < 2)
        return;

    // 1. Host Transport & Tempo Tracking
    if (auto* currentPlayHead = getPlayHead())
    {
        if (auto pos = currentPlayHead->getPosition())
        {
            if (pos->getBpm())
            {
                const double bpm = *pos->getBpm();
                currentBpm.store (bpm, std::memory_order_relaxed);
                delayModule.setTempo (bpm);
                repeatModule.setTempo (bpm);
            }

            if (pos->getTimeSignature())
            {
                timeSigNum.store (pos->getTimeSignature()->numerator, std::memory_order_relaxed);
                timeSigDen.store (pos->getTimeSignature()->denominator, std::memory_order_relaxed);
            }

            hostPlaying.store (pos->getIsPlaying(), std::memory_order_relaxed);
        }
    }

    // 2. Read Parameters Lock-Free
    const float mEnergy   = pMacroEnergy   ? pMacroEnergy->load (std::memory_order_relaxed)   : 0.0f;
    const float mSpace    = pMacroSpace    ? pMacroSpace->load (std::memory_order_relaxed)    : 0.0f;
    const float mMovement = pMacroMovement ? pMacroMovement->load (std::memory_order_relaxed) : 0.0f;
    const float mWild     = pMacroWild     ? pMacroWild->load (std::memory_order_relaxed)     : 0.0f;

    const bool  bFilterEnabled = pFilterEnabled   ? pFilterEnabled->load (std::memory_order_relaxed) > 0.5f : true;
    const float bFilterCutoff  = pFilterCutoff    ? pFilterCutoff->load (std::memory_order_relaxed)        : 20000.0f;
    const float bFilterRes     = pFilterResonance ? pFilterResonance->load (std::memory_order_relaxed)     : 0.707f;
    const int   bFilterType    = pFilterType      ? static_cast<int> (pFilterType->load (std::memory_order_relaxed)) : 0;

    const bool  bSatEnabled = pSatEnabled ? pSatEnabled->load (std::memory_order_relaxed) > 0.5f : true;
    const float bSatDrive   = pSatDrive   ? pSatDrive->load (std::memory_order_relaxed)         : 0.0f;
    const float bSatMix     = pSatMix     ? pSatMix->load (std::memory_order_relaxed)           : 0.0f;

    const bool  bDelayEnabled  = pDelayEnabled  ? pDelayEnabled->load (std::memory_order_relaxed) > 0.5f : true;
    const int   bDelayDivision = pDelayDivision ? static_cast<int> (pDelayDivision->load (std::memory_order_relaxed)) : 2;
    const float bDelayFeedback = pDelayFeedback ? pDelayFeedback->load (std::memory_order_relaxed)        : 0.35f;
    const float bDelayMix      = pDelayMix      ? pDelayMix->load (std::memory_order_relaxed)             : 0.0f;

    const bool  bReverbEnabled = pReverbEnabled ? pReverbEnabled->load (std::memory_order_relaxed) > 0.5f : true;
    const float bReverbSize    = pReverbSize    ? pReverbSize->load (std::memory_order_relaxed)        : 0.6f;
    const float bReverbDamping = pReverbDamping ? pReverbDamping->load (std::memory_order_relaxed)     : 0.4f;
    const float bReverbMix     = pReverbMix     ? pReverbMix->load (std::memory_order_relaxed)         : 0.0f;

    const bool  bRepeatEnabled  = pRepeatEnabled  ? pRepeatEnabled->load (std::memory_order_relaxed) > 0.5f : true;
    const int   bRepeatDivision = pRepeatDivision ? static_cast<int> (pRepeatDivision->load (std::memory_order_relaxed)) : 2;
    const float bRepeatMix      = pRepeatMix      ? pRepeatMix->load (std::memory_order_relaxed)             : 0.0f;

    const float bMasterMix    = pMasterMix    ? pMasterMix->load (std::memory_order_relaxed)    : 1.0f;
    const bool  bMasterBypass = pMasterBypass ? pMasterBypass->load (std::memory_order_relaxed) > 0.5f : false;

    // 3. Master Bypass Check
    if (bMasterBypass)
        return; // Passthrough without processing

    // 4. Compute Effective Parameters via Macro Coordinator
    const auto eff = LautaParams::MacroManager::compute (
        mEnergy, mSpace, mMovement, mWild,
        bFilterEnabled, bFilterType, bFilterCutoff, bFilterRes,
        bSatEnabled, bSatDrive, bSatMix,
        bDelayEnabled, bDelayDivision, bDelayFeedback, bDelayMix,
        bReverbEnabled, bReverbSize, bReverbDamping, bReverbMix,
        bRepeatEnabled, bRepeatDivision, bRepeatMix,
        bMasterMix, bMasterBypass);

    // 5. Save Dry Copy for Master Mix
    if (dryBuffer.getNumSamples() < numSamples)
        dryBuffer.setSize (2, numSamples, false, false, true);

    dryBuffer.copyFrom (0, 0, buffer, 0, 0, numSamples);
    dryBuffer.copyFrom (1, 0, buffer, 1, 0, numSamples);

    // 6. Signal Chain:
    // Stage A: Filter
    if (eff.filterEnabled)
    {
        filterModule.setType (static_cast<LautaDSP::FilterModule::FilterType> (eff.filterType));
        filterModule.setCutoff (eff.filterCutoff);
        filterModule.setResonance (eff.filterResonance);
        filterModule.process (buffer);
    }

    // Stage B: Saturation
    if (eff.satEnabled)
    {
        saturationModule.setDrive (eff.satDrive);
        saturationModule.setMix (eff.satMix);
        saturationModule.process (buffer);
    }

    // Stage C: Delay
    if (eff.delayEnabled)
    {
        delayModule.setDivision (static_cast<LautaDSP::DelayModule::Division> (eff.delayDivision));
        delayModule.setFeedback (eff.delayFeedback);
        delayModule.setMix (eff.delayMix);
        delayModule.process (buffer);
    }

    // Stage D: Reverb
    if (eff.reverbEnabled)
    {
        reverbModule.setRoomSize (eff.reverbSize);
        reverbModule.setDamping (eff.reverbDamping);
        reverbModule.setMix (eff.reverbMix);
        reverbModule.process (buffer);
    }

    // Stage E: Repeat / Glitch
    if (eff.repeatEnabled)
    {
        repeatModule.setDivision (static_cast<LautaDSP::RepeatModule::Division> (eff.repeatDivision));
        repeatModule.setDecay (eff.repeatDecay);
        repeatModule.setMix (eff.repeatMix);
        repeatModule.process (buffer);
    }

    // Stage F: Master Wet/Dry Crossfade
    if (eff.masterMix < 0.999f)
    {
        auto* wetL = buffer.getWritePointer (0);
        auto* wetR = buffer.getWritePointer (1);
        const auto* dryL = dryBuffer.getReadPointer (0);
        const auto* dryR = dryBuffer.getReadPointer (1);

        const float wetGain = eff.masterMix;
        const float dryGain = 1.0f - eff.masterMix;

        for (int i = 0; i < numSamples; ++i)
        {
            wetL[i] = dryL[i] * dryGain + wetL[i] * wetGain;
            wetR[i] = dryR[i] * dryGain + wetR[i] * wetGain;
        }
    }

    // Stage G: Output Safety Limiter (guarantees safe output levels)
    safetyLimiter.process (buffer);
}

bool LautaFXAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* LautaFXAudioProcessor::createEditor()
{
    return new LautaFXAudioProcessorEditor (*this);
}

void LautaFXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LautaFXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LautaFXAudioProcessor();
}
