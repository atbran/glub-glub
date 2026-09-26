# Changelog

## Unreleased (2.1)

- **Camera punch at max hype.** When hype is near the top, the tank gently zooms in on
  Glub and bumps in on every beat (about 2.5% at rest, 6% on the beat). The HUD stays
  still, and gentle mode turns it off.
- **Hype sensitivity slider** (0.5x–2x): how loud the music must be to fill the HYPE
  meter, separate from the "vibe" dance sensitivity.
- **Worm mode:** a toggle that makes about half of his automatic moves the worm (about
  five times its usual share), dancing every bar. Switching it on starts a worm right away.
- **Petting:** stroke Glub slowly with the cursor for happy "^" eyes, extra blush,
  hearts and a wag. He pauses dancing to enjoy it.
- **XP and levels:** Glub is now a persistent pet, the same fish in every project and
  DAW. He earns XP from dancing (faster when hyped), eating, petting and breakdancing.
  Level-ups bring confetti and an announcement.
- **Wardrobe unlocks:** party hat (Lv 2), goldfish skin (3), gold chain (4), shubunkin
  skin (5), crown (6), neon skin (8), golden skin (10).
- **Tabbed control panel:** *tank* (sliders, theme, toggles), *dance* (moves + worm
  mode), *glub* (level card and wardrobe). A level badge and XP bar sit on the stand.

## 2.0.0 — 2026-09-26

Glub-Glub 2.0 is a full overhaul: he hears the beat himself, lives in a real tank,
eats, and every dance move has been rebuilt.

### He hears the beat
- **Tempo by ear.** A new tempo tracker (onset autocorrelation for the BPM, a comb
  filter for the downbeat, and a soft phase-locked loop) beat-locks the koi in the
  standalone app or a stopped DAW. It locks within 1.5 BPM from 90 to 174 BPM and
  lets go after silence. A rolling DAW transport still takes priority.
- **Tempo readout** under the HYPE meter: BPM, where it came from (`host` / `by ear`),
  and a light that blinks on the beat.
- **Three-band analysis** (bass / mids / highs) and a bass-only kick detector drive the
  new scenery.

### Dancing
- **Every move rebuilt except the Worm**, using the Worm as the reference. Each move now
  bends the pixel body itself with a smooth envelope and one clear gesture per beat:
  - Barrel Roll: a true roll to belly-up, two beats of upside-down swimming, roll home
  - Spin: C-curl, tuck and pirouette · Flip: C-bend turn with a bigger hop
  - Shuffle: beat-stepped glides with a hop and lean
  - Head Bop: head snaps down on the beat, tail counter-lifts
  - Shimmy: shimmy down, shimmy up, with shivers rippling head to tail
  - Figure Eight: turns to face where he's swimming and pitches with the path
  - Twerk: sharper on-beat bounce · Breakdance: a visible headspin
- **New moves:** Breakdance (headspin to freeze), Loop-de-loop (with a bubble wake),
  Moonwalk (slides backwards while his tail swims forwards) and a dolphin Tail Walk
  with splashes. 13 moves in total.
- **Max hype = showstoppers.** Hype pegged for 3 s unlocks an intense set every bar
  (loop, tail walk, roll, twerk, spin, worm), plus a chance to breakdance on his own
  (at most every 16 s). Moderate hype never breakdances.
- Moves never collapse to an invisible sliver mid-turn.

### The tank
- **A living pixel-art tank:** dithered water, sand, rocks, coral and a sunken chest.
  Seaweed pumps with the bass and snaps on kicks, coral glows with the mids, and light
  rays and plankton shimmer with the highs. The chest burps a bubble every 8th kick.
- **Themes:** Lagoon, Midnight (bioluminescent), Sunset. The hue slider still fine-tunes.
- **Tank mates:** a school of neon tetras that wander, dodge the koi and scatter on kicks.
- **Steadier disco ball:** it needs sustained hype to drop in, stays at least 8 s, and
  leaves only after a real lull.

### Feeding
- Bigger, outlined food flakes sink and settle on the sand.
- Glub turns around, swims his mouth to each flake and gulps it: open-mouth chomp,
  love hearts, a rounder belly the more he eats, and opinions about the flavour.
- Dancing waits until dinner is done.

### Interface
- **New control panel** in chunky pixel style. It slides up over the tank without
  resizing it and has one-click buttons for all 13 moves (the playing move lights up),
  theme buttons and a tank-mates toggle. Click the water to close it.
- The tank sits on a wooden stand bar showing the version.
- Glub comments when he locks onto a tempo, while eating, and when he's full.

### Fixes and performance
- The tail fan no longer breaks into a dark outline lattice at full energy.
- The koi sprite buffer shrank from window-sized to grid-sized, cutting the cost of
  every frame. A full 1024×1024 frame renders in about 11–13 ms.
- Moves are now a single state machine instead of six flags.

### Developer
- `DspChecks`: tempo accuracy and phase at 90–174 BPM (44.1 / 48 kHz), silence release,
  noise rejection, band levels and the kick detector.
- `KoiMotionChecks`: grows to 59 checks, adding food chasing, tail solidity, max-hype
  choreography and disco stability.
- `EditorShots`: renders the real processor and editor offscreen on a synthetic groove
  (every theme, feeding, the panel, a hero shot and GIF frames), and times a
  1024×1024 frame.

## 1.x

The original Glub-Glub: transparent VST3 / standalone, host-beat-locked bob, nine dance
moves, disco ball, RGB party tint, bubbles, sleepy mode, tiered speech, HYPE meter,
Deal With It glasses and the food shaker.
