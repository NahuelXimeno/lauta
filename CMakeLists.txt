cmake_minimum_required(VERSION 3.22)
project(LAUTA_FX VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE (local external directory or auto-fetch from GitHub)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    add_subdirectory(external/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        7.0.12
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

set(LAUTA_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND LAUTA_FORMATS AU)
endif()

juce_add_plugin(LautaFX
    COMPANY_NAME "Lauta Audio"
    BUNDLE_ID "com.lautaaudio.lautafx"
    PLUGIN_MANUFACTURER_CODE Laut
    PLUGIN_CODE Ltau
    FORMATS ${LAUTA_FORMATS}
    PRODUCT_NAME "LAUTA FX"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(LautaFX PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/Common/Parameters.h
    Source/Common/MacroManager.h
    Source/Common/LautaLookAndFeel.h
    Source/DSP/FilterModule.h
    Source/DSP/SaturationModule.h
    Source/DSP/DelayModule.h
    Source/DSP/ReverbModule.h
    Source/DSP/RepeatModule.h
    Source/DSP/SafetyLimiter.h
)

target_compile_definitions(LautaFX PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(LautaFX PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)

# Automated Test Suite
add_executable(LautaFX_Tests
    Tests/DSPTests.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_include_directories(LautaFX_Tests PRIVATE Source)

target_compile_definitions(LautaFX_Tests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JucePlugin_Name="LAUTA FX"
)

target_link_libraries(LautaFX_Tests PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
