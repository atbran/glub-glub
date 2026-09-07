# Glub-Glub 🐟

Cute pixel koi fish VST3 + Standalone. Transparent audio passthrough — any audio going through makes him dance. Dance fits the vibe (chill sway / hype bounce / sparkle spin + beat lock when the host gives BPM).

- Orange-red procedural pixel koi, 500x500 resizable tank
- Idle mouth bubbles, tiered fish-pun speech (low/med/high/idle, every 30-90s, configurable)
- VST3 + Windows Standalone `.exe` from one JUCE 8 codebase

## Build (Windows + MSVC)
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
Outputs: `build/GlubGlub_artefacts/Release/VST3/Glub-Glub.vst3`, `build/GlubGlub_artefacts/Release/Standalone/Glub-Glub.exe`

Copy `.vst3` to `C:\Program Files\Common Files\VST3\` for Ableton / FL / Reaper.

## Use
- Insert as VST effect on any track — audio passes through untouched, glub-glub dances.
- Standalone: run exe, pick input, play music.
- Bottom drawer: speech rate 30-90s, vibe sensitivity, tank hue, bubbles toggle (saved per instance).

## Vibe engine
RMS energy + onset flux + brightness (ZCR/HF proxy) → Low/Med/High. Host BPM via `AudioPlayHead` (Ableton) → beat-phase bounce + bar-4 spin, smoothed 200ms. Standalone falls back to free-dance.
