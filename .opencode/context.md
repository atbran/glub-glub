# Project Context

## Environment
- Language: C++17 (JUCE 8, via CMake FetchContent)
- Runtime: Windows win32, PowerShell 5.1
- Build: CMake >= 3.22 + MSVC (Visual Studio 2022) / Ninja; `scripts/build-msvc-ninja.bat`
- Test: No test framework yet; verification = CMake configure + build + pluginval smoke (if available)
- Package Manager: CMake FetchContent for JUCE (no local install needed)
- Tools available: cmake (scoop shim), cargo, node v24.19.0, npm 11.17.0
- Git: local git repository on `master`; GitHub remote intentionally not configured yet

## Project Type
- [x] Application (Audio Plugin + Standalone)
- Plugin formats: VST3 + Standalone (JUCE `juce_add_plugin`)
- Effect type: transparent audio passthrough (no DSP on audio) + sidechain visualizer
- Goal: cute pixel koi fish "glub-glub", orange-red, dances to audio vibe, idle bubbles, random cute speech

## Infrastructure
- Container: None
- Orchestration: None
- CI/CD: None
- Cloud: None

## Structure (TO BE CREATED by Worker)
- Source: `Source/` (PluginProcessor.{h,cpp}, PluginEditor.{h,cpp}, DSP/AudioFeatures.{h,cpp}, UI/KoiFish.{h,cpp}, UI/PixelFishLookAndFeel or sprites)
- Tests: none yet (build verification only)
- Docs: `.opencode/docs/` (Planner to cache JUCE docs)
- Entry: `Source/PluginProcessor.cpp` (processor), `Source/PluginEditor.cpp` (UI)
- Build: `CMakeLists.txt` (juce_add_plugin, juce_generate_juce_header)

## Conventions
- Naming: PascalCase classes, camelCase methods (JUCE style)
- Imports: `#include <juce_audio_processors/juce_audio_processors.h>`, `#include <juce_gui_basics/juce_gui_basics.h>`
- Error handling: JUCE assertions + safe defaults (never crash host)
- Pixel art: procedural pixel-grid rendering (no external assets) for crisp scaling
- DSP: lock-free FIFO processor->editor, atomic vibe params, 60fps timer, no allocations on audio thread

## Notes
- Dance must "fit the vibe": map RMS energy + onset/beat + spectral brightness to dance intensity/style (chill sway vs hype bounce vs sparkle spin)
- Idle: mouth bubbles (particle system), occasional speech textbox with cute phrases
- Transparent passthrough: copy input to output unchanged; support arbitrary channels
- Standalone + VST3 from single JUCE target
- Keep CPU low, host-safe, resizable editor ~ 480x480 min
