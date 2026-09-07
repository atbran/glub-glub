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

## M7: Party tier | status: completed
- [x] Hype metric x1.15 sensitivity; disco ball in at 0.45, RGB tint ramp from 0.70
- [x] Disco ball: checker light/dark facets, slowly rotating pattern, sweeping specular band, rim star sparkles, drop-in bounce, no swing
- [x] RGB tint: discrete hue jump per host beat via AudioPlayHead phase; smooth drift fallback when no BPM
- [x] Beat bob doubled in intensity; tail-spin gated to every 4th bar; flips drop-only (pulse>0.92 + energy>0.5, 4s cooldown)
- [x] Idle bob +15%

## M8: Layout polish | status: completed
- [x] Speech box moved to bottom strip (tail points up at fish), can no longer cover the disco ball
- [x] HYPE meter moved to top-right corner
- [x] Default speech rate 60s (~once per minute)

## M9: Feel + disco polish | status: completed
- [x] Beat bob fixed to one dip per quarter note (was double-rate |sin|), hits ON the beat
- [x] Disco ball repositioned to center-top; 100ms/200ms hype debounce on drop/retract
- [x] Fish re-centered (sway reduced, cy lowered), another ~15% global slowdown
- [x] Tail swim 40% slower
- [x] Disco facet rotation/specular/sparkles slowed

## M10: Timing + rendering fixes | status: completed
- [x] Fish rendered via offscreen image (kills rotated-rect AA seams / "segmented fish")
- [x] Pulse hop suppressed to 25% when host BPM is live (bob stays on-grid at high hype)
- [x] Disco ball raised to window top (clears fish), fish ride height lowered slightly
