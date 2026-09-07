# Glub-Glub 🐟

Cute pixel koi fish VST3 + Standalone. Transparent audio passthrough — any audio going through makes him dance. Dance fits the vibe (chill sway / hype bounce / sparkle spin + beat lock when the host gives BPM).

- Kohaku-style procedural pixel koi, 500x500 resizable tank
- Shaded near-3D pixel look: bendy spine body, connected fan tail, 3-tone shading, sumi spots
- Dance moves: traveling undulation, strong beat bob (host-BPM locked), bar tail-spins, flip on drops, 8-bar party spin
- Party tier: pixel disco ball (rotating facets, sweeping specular, rim star sparkles) at medium hype; beat-stepped RGB water tint at 70%+ hype (smooth drift when no host BPM)
- Beat bubble-bursts from his mouth at high hype; HYPE meter top-right; cute speech box at the bottom (~once per minute default)
- Idle mouth bubbles, tiered fish-pun speech (low/med/high/idle, every 30-90s, configurable)
- VST3 + Windows Standalone `.exe` from one JUCE 8 codebase

## Build (Windows + MSVC)
The reliable build entry point is the MSVC + Ninja script. It initializes the Visual Studio toolchain explicitly and avoids a CMake/Visual Studio generator probe issue on some installations:
```cmd
scripts\build-msvc-ninja.bat
```

Manual equivalent:
```powershell
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2022\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
"%ProgramFiles%\CMake\bin\cmake.exe" --fresh -S . -B build-msvc -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe
"%ProgramFiles%\CMake\bin\cmake.exe" --build build-msvc --parallel
```
Outputs: `build-msvc/GlubGlub_artefacts/Release/VST3/Glub-Glub.vst3`, `build-msvc/GlubGlub_artefacts/Release/Standalone/Glub-Glub.exe`

Copy `.vst3` to `C:\Program Files\Common Files\VST3\` for Ableton / FL / Reaper.

## Use
- Insert as VST effect on any track — audio passes through untouched, glub-glub dances.
- Standalone: run exe, pick input, play music.
- Bottom drawer: speech rate 30-90s, vibe sensitivity, tank hue, bubbles toggle (saved per instance).

## Vibe engine
RMS energy + onset flux + brightness (ZCR/HF proxy) → Low/Med/High. Host BPM via `AudioPlayHead` (Ableton) → beat-phase bounce + bar-4 spin, smoothed 200ms. Standalone falls back to free-dance.
