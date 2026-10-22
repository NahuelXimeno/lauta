#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_audio_processors/juce_audio_processors.h>

#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

#include <iostream>
#include <cassert>
#include <cmath>

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "FAIL: " << msg << " (" << #cond << ") at line " << __LINE__ << std::endl; \
            return false; \
        } \
    } while (0)

bool testInstantiation()
{
    std::cout << "[TEST] 1. Instantiation and Bus Layout..." << std::endl;
    LautaFXAudioProcessor processor;

    TEST_ASSERT (processor.getName() == "LAUTA FX", "Plugin name must be LAUTA FX");
    TEST_ASSERT (processor.hasEditor(), "Plugin must have an editor");
    TEST_ASSERT (processor.getNumPrograms() == 4, "Must have 4 factory programs (INIT, GROOVE, BUILD, BREAKDOWN)");

    juce::AudioProcessor::BusesLayout stereoLayout;
    stereoLayout.inputBuses.add (juce::AudioChannelSet::stereo());
    stereoLayout.outputBuses.add (juce::AudioChannelSet::stereo());
    TEST_ASSERT (processor.isBusesLayoutSupported (stereoLayout), "Stereo bus layout must be supported");

    juce::AudioProcessor::BusesLayout monoLayout;
    monoLayout.inputBuses.add (juce::AudioChannelSet::mono());
    monoLayout.outputBuses.add (juce::AudioChannelSet::mono());
    TEST_ASSERT (! processor.isBusesLayoutSupported (monoLayout), "Mono layout should be rejected (stereo dedicated)");

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testStereoSignalFlow()
{
    std::cout << "[TEST] 2. Stereo Signal Flow..." << std::endl;
    LautaFXAudioProcessor processor;
    processor.prepareToPlay (48000.0, 512);

    juce::AudioBuffer<float> buffer (2, 512);
    for (int i = 0; i < 512; ++i)
    {
        buffer.setSample (0, i, 0.4f * std::sin (2.0 * juce::MathConstants<double>::pi * 440.0 * i / 48000.0));
        buffer.setSample (1, i, 0.2f * std::cos (2.0 * juce::MathConstants<double>::pi * 880.0 * i / 48000.0));
    }

    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);

    float rmsL = buffer.getRMSLevel (0, 0, 512);
    float rmsR = buffer.getRMSLevel (1, 0, 512);

    TEST_ASSERT (rmsL > 0.001f, "Left channel should have signal");
    TEST_ASSERT (rmsR > 0.001f, "Right channel should have signal");
    TEST_ASSERT (std::abs (rmsL - rmsR) > 0.01f, "Stereo channels should maintain distinct characteristics");

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testSilenceHandling()
{
    std::cout << "[TEST] 3. Silence and Near-Silence..." << std::endl;
    LautaFXAudioProcessor processor;
    processor.prepareToPlay (44100.0, 256);

    juce::AudioBuffer<float> buffer (2, 256);
    buffer.clear();
    juce::MidiBuffer midi;

    // Process multiple blocks of absolute silence
    for (int b = 0; b < 10; ++b)
        processor.processBlock (buffer, midi);

    float maxPeak = buffer.getMagnitude (0, 256);
    TEST_ASSERT (maxPeak < 1e-6f, "Zero input must produce zero output (no DC or spurious noise)");

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testBypassBehavior()
{
    std::cout << "[TEST] 4. Bypass and Reset Behavior..." << std::endl;
    LautaFXAudioProcessor processor;
    processor.prepareToPlay (48000.0, 256);

    auto* bypassParam = processor.getAPVTS().getParameter (LautaParams::masterBypass.getParamID());
    TEST_ASSERT (bypassParam != nullptr, "Bypass parameter must exist");
    bypassParam->setValueNotifyingHost (1.0f); // Engage bypass

    juce::AudioBuffer<float> input (2, 256);
    juce::AudioBuffer<float> buffer (2, 256);
    for (int i = 0; i < 256; ++i)
    {
        float val = static_cast<float> (i) / 256.0f;
        input.setSample (0, i, val);
        input.setSample (1, i, -val);
        buffer.setSample (0, i, val);
        buffer.setSample (1, i, -val);
    }

    juce::MidiBuffer midi;
    processor.processBlock (buffer, midi);

    // In bypass, buffer should be completely unaltered
    for (int i = 0; i < 256; ++i)
    {
        TEST_ASSERT (buffer.getSample (0, i) == input.getSample (0, i), "Bypass must pass audio untouched (L)");
        TEST_ASSERT (buffer.getSample (1, i) == input.getSample (1, i), "Bypass must pass audio untouched (R)");
    }

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testParameterBoundariesAndFiniteValues()
{
    std::cout << "[TEST] 5. Parameter Boundaries & Finite Output..." << std::endl;
    LautaFXAudioProcessor processor;
    processor.prepareToPlay (44100.0, 128);

    auto& apvts = processor.getAPVTS();
    auto* pEnergy = apvts.getParameter (LautaParams::macroEnergy.getParamID());
    auto* pSpace  = apvts.getParameter (LautaParams::macroSpace.getParamID());
    auto* pMove   = apvts.getParameter (LautaParams::macroMovement.getParamID());
    auto* pWild   = apvts.getParameter (LautaParams::macroWild.getParamID());

    // Sweep all 4 macros through extreme values
    float testValues[] = { 0.0f, 0.5f, 1.0f, 0.0f, 1.0f, 0.25f, 0.75f };

    juce::AudioBuffer<float> buffer (2, 128);
    juce::MidiBuffer midi;

    for (float val : testValues)
    {
        pEnergy->setValueNotifyingHost (val);
        pSpace->setValueNotifyingHost (val);
        pMove->setValueNotifyingHost (val);
        pWild->setValueNotifyingHost (val);

        // Fill with sine test signal
        for (int i = 0; i < 128; ++i)
        {
            buffer.setSample (0, i, 0.5f * std::sin (i * 0.1f));
            buffer.setSample (1, i, 0.5f * std::cos (i * 0.1f));
        }

        processor.processBlock (buffer, midi);

        for (int ch = 0; ch < 2; ++ch)
        {
            for (int i = 0; i < 128; ++i)
            {
                float s = buffer.getSample (ch, i);
                TEST_ASSERT (std::isfinite (s), "Output sample must be finite (no NaN or Inf)");
                TEST_ASSERT (std::abs (s) <= 1.05f, "Output sample must not exceed safety limit");
            }
        }
    }

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testStateSaveAndRestore()
{
    std::cout << "[TEST] 6. State Save and Restore (Persistence)..." << std::endl;
    LautaFXAudioProcessor procA;
    LautaFXAudioProcessor procB;

    auto& apvtsA = procA.getAPVTS();
    auto& apvtsB = procB.getAPVTS();

    // Set distinctive values on Proc A
    apvtsA.getParameter (LautaParams::macroEnergy.getParamID())->setValueNotifyingHost (0.77f);
    apvtsA.getParameter (LautaParams::macroSpace.getParamID())->setValueNotifyingHost (0.64f);
    apvtsA.getParameter (LautaParams::macroMovement.getParamID())->setValueNotifyingHost (0.42f);
    apvtsA.getParameter (LautaParams::macroWild.getParamID())->setValueNotifyingHost (0.88f);
    apvtsA.getParameter (LautaParams::satDrive.getParamID())->setValueNotifyingHost (0.50f);

    juce::MemoryBlock memBlock;
    procA.getStateInformation (memBlock);

    TEST_ASSERT (memBlock.getSize() > 0, "Serialized state must not be empty");

    // Restore to Proc B
    procB.setStateInformation (memBlock.getData(), static_cast<int> (memBlock.getSize()));

    float valEnergyB = apvtsB.getParameter (LautaParams::macroEnergy.getParamID())->getValue();
    float valSpaceB  = apvtsB.getParameter (LautaParams::macroSpace.getParamID())->getValue();
    float valMoveB   = apvtsB.getParameter (LautaParams::macroMovement.getParamID())->getValue();
    float valWildB   = apvtsB.getParameter (LautaParams::macroWild.getParamID())->getValue();

    TEST_ASSERT (std::abs (valEnergyB - 0.77f) < 0.01f, "Restored ENERGY parameter must match");
    TEST_ASSERT (std::abs (valSpaceB  - 0.64f) < 0.01f, "Restored SPACE parameter must match");
    TEST_ASSERT (std::abs (valMoveB   - 0.42f) < 0.01f, "Restored MOVEMENT parameter must match");
    TEST_ASSERT (std::abs (valWildB   - 0.88f) < 0.01f, "Restored WILD parameter must match");

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testFeedbackStability()
{
    std::cout << "[TEST] 7. Feedback Stability Under Runaway Conditions..." << std::endl;
    LautaFXAudioProcessor processor;
    processor.prepareToPlay (44100.0, 512);

    auto& apvts = processor.getAPVTS();
    // Max feedback and high delay mix
    apvts.getParameter (LautaParams::delayFeedback.getParamID())->setValueNotifyingHost (0.95f);
    apvts.getParameter (LautaParams::delayMix.getParamID())->setValueNotifyingHost (0.8f);
    apvts.getParameter (LautaParams::satDrive.getParamID())->setValueNotifyingHost (0.8f);

    juce::AudioBuffer<float> buffer (2, 512);
    juce::MidiBuffer midi;

    // Send high-energy pulse
    buffer.clear();
    buffer.setSample (0, 0, 1.0f);
    buffer.setSample (1, 0, -1.0f);
    processor.processBlock (buffer, midi);

    // Now process 100 blocks of silence to simulate long feedback cycling
    for (int b = 0; b < 100; ++b)
    {
        buffer.clear();
        processor.processBlock (buffer, midi);

        float peakL = buffer.getMagnitude (0, 512);
        float peakR = buffer.getMagnitude (1, 512);

        TEST_ASSERT (std::isfinite (peakL) && std::isfinite (peakR), "Feedback samples must remain finite");
        TEST_ASSERT (peakL < 1.05f && peakR < 1.05f, "Feedback signal must not runaway or blow up");
    }

    std::cout << "  -> PASS" << std::endl;
    return true;
}

bool testEditorLifecycle()
{
    std::cout << "[TEST] 8. Repeated Editor Opening & Closing Lifecycle..." << std::endl;
    LautaFXAudioProcessor processor;

    for (int cycle = 0; cycle < 10; ++cycle)
    {
        auto* editor = processor.createEditor();
        TEST_ASSERT (editor != nullptr, "Editor creation must succeed");

        editor->setSize (940, 580);
        juce::Image image (juce::Image::ARGB, 940, 580, true);
        juce::Graphics g (image);
        editor->paintEntireComponent (g, true);

        delete editor;
    }

    std::cout << "  -> PASS" << std::endl;
    return true;
}

int main()
{
    juce::MessageManager::getInstance();

    std::cout << "========================================" << std::endl;
    std::cout << "   LAUTA FX - AUTOMATED DSP TEST SUITE   " << std::endl;
    std::cout << "========================================" << std::endl;

    bool allPassed = true;
    allPassed &= testInstantiation();
    allPassed &= testStereoSignalFlow();
    allPassed &= testSilenceHandling();
    allPassed &= testBypassBehavior();
    allPassed &= testParameterBoundariesAndFiniteValues();
    allPassed &= testStateSaveAndRestore();
    allPassed &= testFeedbackStability();
    allPassed &= testEditorLifecycle();

    std::cout << "========================================" << std::endl;
    if (allPassed)
    {
        std::cout << "   ALL 8 TEST SUITES PASSED!            " << std::endl;
        std::cout << "========================================" << std::endl;
        juce::MessageManager::deleteInstance();
        return 0;
    }
    else
    {
        std::cerr << "   SOME TESTS FAILED!                   " << std::endl;
        std::cout << "========================================" << std::endl;
        juce::MessageManager::deleteInstance();
        return 1;
    }
}
