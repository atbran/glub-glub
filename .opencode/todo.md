# Mission: Glub-Glub VST3 + Standalone

## M1: JUCE scaffold | status: completed
- [x] CMake FetchContent JUCE 8.0.6 and VST3/Standalone targets
- [x] Transparent AudioProcessor passthrough and APVTS parameters

## M2: Audio vibe engine | status: completed
- [x] RMS envelope, onset flux, brightness proxy, and intensity tiers
- [x] Ableton/host BPM and beat phase with standalone fallback

## M3: Pixel koi presentation | status: completed
- [x] Procedural orange-red koi with idle sway, beat bounce, brightness sparkle, and bar spin
- [x] Idle bubbles and tiered speech box with 30-90 second scheduling
- [x] Resizable 500x500 tank and persistent configuration drawer

## M4: Verification | status: completed
- [x] MSVC 19.44 + Ninja Release build passed
- [x] Standalone startup smoke check passed
- [x] VST3 x64 binary and moduleinfo.json generated

## M5: Koi polish pass | status: completed
- [x] Spine-based bendy body with connected fan tail (no more tail gap)
- [x] Near-3D shading: 3-tone body/fin shading, warm outline, sumi spots, eye glint, blush
- [x] Dance moves: undulation, beat squash-hop, bar tail-spin, cooldown-gated flip
- [x] HYPE meter component wired to energy + beat + intensity
- [x] Fixed collapsed-drawer slider leak (controls hidden when collapsed)
- [x] Replaced random sparkle rects with intentional pulsing twinkles

## M6: Pacing + new moves | status: completed
- [x] Global animation slowdown ~25% (undulation, wag, bob, sway, flips, spins, twinkles)
- [x] Sleepy ZZZ mode after ~6s silence (closed eye, slower drift, floating Zs)
- [x] Breathing idle (subtle body radius pulse)
- [x] 8-bar eased 360 party spin
- [x] Beat bubble-bursts from mouth at high hype (Bubbles::burst)
- [x] HYPE meter moved to bottom-right (was covering seaweed)
