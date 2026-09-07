cmake_minimum_required(VERSION 3.22)
project(GlubGlub VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.6.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(GlubGlub
  COMPANY_NAME "GlubGlub"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  PLUGIN_MANUFACTURER_CODE Glub
  PLUGIN_CODE Glub
  FORMATS VST3 Standalone
  PRODUCT_NAME "Glub-Glub"
  VERSION 0.1.0
)

target_sources(GlubGlub PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginEditor.cpp
  Source/DSP/AudioFeatures.cpp
  Source/UI/KoiFish.cpp
  Source/UI/Bubbles.cpp
  Source/UI/SpeechBox.cpp
  Source/UI/ConfigDrawer.cpp
)

target_compile_definitions(GlubGlub PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(GlubGlub PRIVATE
  juce::juce_audio_utils
  juce::juce_audio_processors
  juce::juce_gui_basics
  juce::juce_audio_basics
  juce::juce_audio_devices
  juce::juce_core
  juce::juce_data_structures
  juce::juce_graphics
)

juce_generate_juce_header(GlubGlub)
