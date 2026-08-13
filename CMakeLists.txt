cmake_minimum_required(VERSION 3.22)
project(SWATronix2A VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 9.0.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SWATronix2A
    COMPANY_NAME "StreetWave Audio"
    BUNDLE_ID "com.streetwaveaudio.swatronix2a"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE StWa
    PLUGIN_CODE Sw2A
    FORMATS VST3 Standalone
    PRODUCT_NAME "SWA Tronix 2A"
)

target_sources(SWATronix2A PRIVATE
    src/PluginProcessor.cpp
    src/PluginProcessor.h
    src/PluginEditor.cpp
    src/PluginEditor.h
)

target_compile_definitions(SWATronix2A PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SWATronix2A PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_extra
)

if(MSVC)
    target_compile_options(SWATronix2A PRIVATE /W4 /permissive-)
else()
    target_compile_options(SWATronix2A PRIVATE -Wall -Wextra -Wpedantic)
endif()
