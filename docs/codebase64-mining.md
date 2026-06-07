# codebase64.net Mining Notes — for goattracker2 Qt frontend

Mining target: `base:sid_programming` and its child pages. Goal:
identify presets, table templates, decoder branches, and docs worth
landing in the Qt build, without duplicating what already ships.

---

## 1. Source map

- **base:sid_programming** — Hub; mostly a link taxonomy.
- **base:triangle_waveform** — Asger Alstrup. Tri = `$10`, test = `$08`,
  internal 512-step counter mirrored about 256.
- **base:noise_waveform** — 23-bit LFSR init `$7FFFF8`, period ≈ 8 MB;
  test-bit reset is delayed `$2000-$8000` cycles (matters when cleaning
  combined-noise lock).
- **base:classic_hard-restart_and_about_adsr_in_generally** — 15-bit
  prescaler, bug window ≈32,768 cycles (~1.7 frames @ 50 Hz). Example HR
  ADSR pairs `$0000`, `$7707`, `$FF0F`.
- **base:a_new_kind_of_hard-restart** — Lft / Linus Åkesson's rate-counter
  recapture: drives RC to a known value via attack/decay micro-pings +
  overflow exploit, ~10 raster lines. Principle (HR is also about RC,
  not just env=$00) is doc-worthy.
- **base:formant** — Vowel formant tables for tri-voice formant synth:
  A=50/102/240, E=28/152/232, I=11/182/255, O=34/90/210, U=15/105/200.
  Tri (`$11`) when voiced > fricative, else noise (`$81`). F0 via $D418
  toggle at 200 Hz.
- **base:reduce_noise** — Software gem: write `$08` to $D417 with all
  filter types off → kills external AIN. Also `sta $d011` mutes VIC.
- **base:nmi_sample_player** — $D418 4/8-bit player. CIA-2 timer at
  $DD04/$DD05. Digiboost = SR=$F0 across all voices.
- **base:detecting_sid_type** — 6581 vs 8580: write $02→$D40F, $30→$D412,
  poll $D41B bit 7 with timeout. Engine-only.
- **base:pal_frequency_table** + **base:how_to_calculate_your_own_sid_frequency_table**
  — PAL constant ≈ 17.03 (≈985 kHz clock), NTSC ≈ 1.023 MHz. Pitch
  offset between PAL/NTSC is nonlinear.
- **base:playing_music_on_pal_and_ntsc** — Timer adjusts but pitch
  can't be re-mapped linearly. Engine-level.
- **base:simple_irq_music_player** — Standard raster IRQ template; ACK
  $D019, mask $D01A. Nothing tracker-new.
- **base:very_short_sid_playroutine** — Fast-forward by skipping raster
  wait while spacebar held; could inform an "audition" mode.
- **base:microtracker_v1.0** — Syndrom's 6-rasterline tracker.
  Per-voice format: V1 drums, V2 pulse+vibrato lead, V3 filter bass.
  *Channel role* presets are a UX idea.
- **base:256_bytes_tune_player** — AD-only envelopes (SR=$00 default).
  Minimum viable instrument = AD + waveform pair.
- **base:digis_r_eazy** + **base:vicious_sid_demo_routine_explained** —
  Vicious SID's killer trick: waveform `$00` *sustains* the DAC level
  for seconds; lets one voice play-and-seek a 4-bit sample. Works on
  both 6581 and 8580.
- **base:building_a_music_routine** — Defines goattracker's lineage:
  wavetable byte-pair convention (waveform + note, relative arp /
  absolute drum) and pulsetable/filtertable "add `<m>` for `<n>` frames"
  commands.
- **DEAD pages** (all link off `base:sid_programming` or `base:start` but
  return "topic does not exist"): `base:reading_the_paddles`,
  `base:sid_voice_3_used_for_randomness`, `base:vibrato_routine`,
  `base:detecting_sid_chip_model` (live page is `base:detecting_sid_type`),
  `base:basic_sid_distortion`, `base:wavetable_synth_routine`,
  `base:howto_filtering`, `base:hard_restart`, `base:filter_tips`,
  `base:pulse_width_modulation`, `base:arpeggios`, `base:sid_tracker`,
  `base:advanced_irq_music_player`, `base:effective_use_of_voice_3`,
  `base:filter_and_resonance`, `base:demo_programming`. The codebase64 SID
  index over-promises; many "see also" links never had content written.

---

## 2. Instrument-preset proposals

ADSR packed `ad/sr` per goattracker convention; `firstwave` $09 =
gate+test (default). Wavetable rows are `L R` byte pairs.

| name | ADSR | first-frame wave | wavetable program (rows of L R) | pulsetable | filtertable | source URL |
|------|------|------------------|---------------------------------|------------|-------------|------------|
| Brass-pluck (Hubbard saw+pulse) | A=0 D=A S=8 R=8 (`0A 88`) | $09 | `61 00` saw+pulse / `21 00` saw / `FF 02` loop step 2 | — | optional LP $90 cutoff $80 res $4 mask 1 | base:building_a_music_routine, base:sid_programming combos |
| Hard-kick (NMI-style digiboost on synth voice) | A=0 D=2 S=0 R=0 (`02 00`) | $09 | `81 D8` noise high / `81 C0` noise mid / `41 A0` pulse low / `41 80` pulse very low / `FF 00` | $80 00 (open pulse) | LP $90 cutoff $30 res $4 mask 1 | base:nmi_sample_player, base:digis_r_eazy (sub-voice analog of $D418 thump) |
| Ringmod-bell (Hubbard Central Park) | A=0 D=E S=2 R=A (`0E 2A`) | $09 | `15 00` tri+ring carrier / `FF 01` hold | — | optional BP $A0 cutoff $C0 res $F mask 1 | base:sid_programming + already in sid-register-tricks §4 |
| Sync-lead (Tel-style robot voice) | A=0 D=8 S=A R=4 (`08 A4`) | $09 | `23 00` saw+sync+gate / `FF 01` hold | $84 00 (narrow pulse, when used with pulse base) | — | base:sid_programming combos, sid-register-tricks §4 |
| Noise-snare (multi-trigger with test bit) | A=0 D=8 S=0 R=4 (`08 04`) | $09 | `81 D0` noise abs / `15 AA` tri+ring abs / `41 A4` pulse abs / `09 00` test+gate restart / `81 D4` noise again / `FF 00` | — | HP $C0 cutoff $A0 res $4 mask 1 | base:noise_waveform (LFSR lock + test recovery) + sid-register-tricks §4 |
| Vocal-pad "AH" (formant) | A=8 D=4 S=E R=8 (`84 E8`) | $09 | `11 00` triangle / `FF 01` hold | — | BP $A7 cutoff $66 res $7 mask 7 (F1≈50 mapped to cutoff $66 from formant table) | base:formant |
| Vocal-pad "OO" (formant) | A=8 D=4 S=E R=8 (`84 E8`) | $09 | `11 00` triangle / `FF 01` hold | — | BP $A7 cutoff $22 res $7 mask 7 (F1≈15 → cutoff $22) | base:formant |
| Organ-pulse (Galway saw+pulse) | A=0 D=0 S=F R=2 (`00 F2`) | $09 | `61 00` saw+pulse / `FF 01` hold | $88 00 (square) | — | base:sid_programming combos, sid-register-tricks §2 |
| Bagpipe-drone (combined wave, no release) | A=0 D=0 S=F R=0 (`00 F0`) | $09 | `51 00` tri+pulse / `FF 01` hold | $84 00 narrow / `40 02` slow +2 PWM drift / `40 FE` slow -2 / `FF 02` loop | — | base:sid_programming combos ("breathy 8580 lead") + sid-register-tricks §2 |
| PWM-ping (Galway/Wizball lead) | A=0 D=6 S=A R=6 (`06 A6`) | $09 | `41 00` pulse / `FF 01` hold | $88 00 set / `02 20` fast +32 / `02 E0` fast -32 / `FF 02` loop | — | base:sid_programming combos + sid-register-tricks §1-3 |
| Digi-tom (test-bit double hit) | A=0 D=A S=0 R=2 (`0A 02`) | $09 | `41 90` pulse abs low / `09 00` test+gate / `41 88` pulse abs lower / `FF 00` | — | LP $90 cutoff $40 res $8 mask 1 | base:vicious_sid_demo_routine_explained, base:noise_waveform |

(11 entries; cherry-pick. The "Choir EE" variant is implicit in
Vocal-pad — just swap the filter cutoff to $98.)

---

## 3. Table "Add step" templates

New entries for `+ Add step…` in qt/TablesView.cpp.

**Wave:**
- Wave: Triangle + Ring (carrier) — `15 00`, 'Triangle with ring-mod input from prev voice; classic Hubbard bell carrier.'
- Wave: Sawtooth + Sync — `23 00`, 'Sawtooth synced to prev voice; Tel-style robot/lead.'
- Wave: Saw + Pulse combo — `61 00`, 'Galway-style reedy organ; sounds dirtier on 6581, mellower on 8580.'
- Wave: Tri + Pulse combo — `51 00`, 'Breathy flute / wet clarinet; thinner on 6581.'
- Wave: Test bit re-trigger — `09 00`, 'Test+gate: silently restarts oscillator phase for double-hit drums.'
- Wave: Noise abs $D0 (snare top) — `81 D0`, 'Bright noise hit; anchors snare attack pitch.'
- Wave: Noise abs $C0 + tri-ring abs $AA — pair `81 C0` then `15 AA`, 'Snare body: bright noise → ringed metal.' (this is two rows; the existing menu only adds one row, so this template would be a small upgrade — see §5.)
- Wave: Inaudible delay $E1 (1 frame) — `E1 00`, 'Silent placeholder that keeps voice 3 sync alive without audible output.'
- Wave: Arpeggio +07 (perfect 5th) — `00 07`, 'Hold waveform, jump pitch up 7 semitones — chord arp middle note.'
- Wave: Arpeggio +0C (octave) — `00 0C`, 'Hold waveform, jump up 12 semitones — chord arp top note.'

**Pulse:**
- Pulse: Set narrow $200 (thin) — `82 00`, 'Buzzy thin pulse; combine with PWM sweep for synth lead.'
- Pulse: Set $C00 (inverse narrow) — `8C 00`, 'Wide inverse-narrow pulse; richer than $800 square.'
- Pulse: Slow drift ±2 — pair `40 02` and `40 FE`, 'Slow PWM drift up then down — bagpipe-style swelling pulse.'
- Pulse: Galway-fast ±32 — pair `02 20` and `02 E0`, 'Aggressive fast PWM, 2-tick reversals; "Wizball" lead signature.'

**Filter:**
- Filter: Lowpass kick — `90 14`, 'LP, res 1, ch1 only — thump body for synth kicks.'
- Filter: Highpass hihat — `C0 04`, 'HP, no res, ch3 only — clears low rumble from noise hat.'
- Filter: Vocal "AH" formant — `A7 66`, 'Bandpass, res 7, all ch — F1 ≈ vowel A (codebase64 formant table).'
- Filter: Vocal "OO" formant — `A7 22`, 'Bandpass, res 7, all ch — F1 ≈ vowel U.'
- Filter: Vocal "EE" formant — `A7 98`, 'Bandpass, res 7, all ch — F1 ≈ vowel I.'
- Filter: Slow res sweep —1 — `7F FF`, 'Gentle descending cutoff sweep for dub-style tails (already present).' (already present – mention to verify.)
- Filter: External-in OFF safety — `80 08`, 'No filter mode, ext-input bit only — cleans up unused AIN hum (base:reduce_noise).'

**Speed (vibrato/portamento):**
- Speed: Slow LFO speed 2 / depth $02 — `02 02`, 'Gentle vibrato — Hubbard ballad style.'
- Speed: Note-indep divisor 8 — `88 04`, 'Note-independent vibrato; constant Hz regardless of pitch.'
- Speed: Portamento $0080 / tick — `00 80`, 'Slide pitch fast; ≈ semitone per few frames at A4.'

---

## 4. Cell decoder additions

New branches for `decodeCell()` in qt/TablesView.cpp.

- **Wave $09 (test+gate).** Append `(silent oscillator restart — drum
  double-hits)`. Source: noise_waveform test-bit latency.
- **Wave $C1 / $A1 / $91 (noise + tri/saw/pulse).** Locks the LFSR.
  Append `WARN: locks noise LFSR — recover with $09 then $81`.
  Source: noise_waveform.
- **Wave $00 with non-zero R.** Append `(DAC sustain — voice silent
  but holds last output; Vicious SID trick)`. Source:
  vicious_sid_demo_routine_explained.
- **Wave $E0.** Today: `inaudible $00`. Append `(silent placeholder —
  keeps V3 oscillator alive for ring/sync targets)`.
- **Pulse $80 00 (PW $000).** Append `(near-DC, audible only with
  modulation)`.
- **Pulse $8F FF (PW $FFF).** Append `(maximum width, symmetric to
  $000, also near-silent)`.
- **Filter $8X R with R bit 3 set.** Append `(external-input bit on —
  feeds AIN pin, usually unwanted hum)`. Source: reduce_noise.
- **Filter `$X0` with ch-mask 0.** Append `(no voices routed — mode
  set but inaudible)`. Saves a debug round-trip.
- **Speed table, note-indep + divisor 0.** Append `(divide-by-zero —
  engine treats as 1)`.
- **Wave $FF FF.** Append `(jumps past 255-entry table — engine wraps
  to step $01; common typo for $FF 01)`.

---

## 5. New docs to write

- **docs/formant-presets.md** — Translate codebase64's F1/F2/F3 vowel
  tables (A=50/102/240, E=28/152/232, I=11/182/255, O=34/90/210,
  U=15/105/200) into goattracker filter recipes. Approximate each
  vowel by sweeping bandpass to the lowest formant (`cutoff ≈ F1 *
  1.7` clipped to $FF). Ready-to-paste wavetable + filtertable + ADSR
  triplets for AH/EH/EE/OH/OO + nasal M/N. Pairs with the existing
  Strings preset to fake a choir.

- **docs/drum-cookbook.md** — Eight drum patches as complete L/R
  wavetable scripts: kick, snare, tom, closed/open hat, ride,
  ringmod-cowbell, test-bit click. Each row annotated with what the
  test bit, noise-after-tri, and abs-note tricks do. Today drum design
  is folklore copy-pasted from old tunes; this would be one page that
  explains *why* each byte sits where it does.

- **docs/chip-detection-and-stereo.md** (or paragraph in
  stereo-port-plan.md) — codebase64's 6581 vs 8580 probe (write
  $02→$D40F, $30→$D412, poll $D41B bit 7). Pairs with our hybrid
  6581+8580 dual-SID layout: song header could tag desired chip and
  StatusStrip.cpp could badge a mismatch. Engine-only change but
  user-visible feedback when the wrong SID is loaded.

- **docs/hard-restart-deep-dive.md** — Extend sid-register-tricks §5
  with the rate-counter-recapture HR from base:a_new_kind_of_hard-restart.
  Even if we don't expose the algorithm itself in the UI, composers
  benefit from understanding the trade: classic HR = 2 frames silent,
  RC-recapture HR = sub-frame at the cost of an audible $F0 SR ping.
  Justifies the HR-preset menu values rationally instead of as folklore.

---

## 6. Skipped (and why)

- **Cycle-exact register write timing** — libresidfp handles it; no
  composer surface.
- **Paddle reading via POTs** — page dead; no live-input path in the
  tracker model anyway.
- **VIC h-sync screen-off / AIN-pin grounding** — real-hardware noise
  mitigations only.
- **Single-voice $D418 digi player** — we're polyphonic; the relevant
  bit (waveform-$00 DAC sustain) already made it into the Digi-tom
  preset.
- **PAL vs NTSC pitch correction** — codebase64 explicitly says it's
  nonlinearly fixable; leave as a song-header tag, not per-note.
- **256-byte AD-only minimal envelope** — covered by existing presets.
- **Asger's bit-level triangle/noise counter analysis** — useful to an
  emulator author, not preset design; relevant bits (LFSR lock,
  test-bit latency) folded into decoder notes above.
- **OSC3/ENV3 readback as randomness/LFO source** — runtime trick the
  speed table already covers more controllably.
- **Raster-IRQ / NMI player templates, digiboost SR=$F0** — engine-level.

---

## Sources

URLs in visit order:

- https://codebase64.net/doku.php?id=base:sid_programming
- https://codebase64.net/doku.php?id=base:triangle_waveform
- https://codebase64.net/doku.php?id=base:noise_waveform
- https://codebase64.net/doku.php?id=base:classic_hard-restart_and_about_adsr_in_generally
- https://codebase64.net/doku.php?id=base:a_new_kind_of_hard-restart
- https://codebase64.net/doku.php?id=base:wavetable_synth_routine (DEAD)
- https://codebase64.net/doku.php?id=base:formant
- https://codebase64.net/doku.php?id=base:reduce_noise
- https://codebase64.net/doku.php?id=base:basic_sid_distortion (DEAD)
- https://codebase64.net/doku.php?id=base:sid_voice_3_used_for_randomness (DEAD)
- https://codebase64.net/doku.php?id=base:vibrato_routine (DEAD)
- https://codebase64.net/doku.php?id=base:nmi_sample_player
- https://codebase64.net/doku.php?id=base:detecting_sid_type
- https://codebase64.net/doku.php?id=base:pal_frequency_table
- https://codebase64.net/doku.php?id=base:256_bytes_tune_player
- https://codebase64.net/doku.php?id=base:digis_r_eazy
- https://codebase64.net/doku.php?id=base:vicious_sid_demo_routine_explained
- https://codebase64.net/doku.php?id=base:reading_the_paddles (DEAD)
- https://codebase64.net/doku.php?id=sid:simple_irq_music_player
- https://codebase64.net/doku.php?id=base:detecting_sid_chip_model (DEAD)
- https://codebase64.net/doku.php?id=base:very_short_sid_playroutine
- https://codebase64.net/doku.php?id=base:microtracker_v1.0
- https://codebase64.net/doku.php?id=base:playing_music_on_pal_and_ntsc
- https://codebase64.net/doku.php?id=base:howto_filtering (DEAD)
- https://codebase64.net/doku.php?id=base:how_to_calculate_your_own_sid_frequency_table
- https://codebase64.net/doku.php?id=base:building_a_music_routine
- https://codebase64.net/doku.php?id=base:sid_tracker (DEAD)
- https://codebase64.net/doku.php?id=base:arpeggios (DEAD)
- https://codebase64.net/doku.php?id=base:pulse_width_modulation (DEAD)
- https://codebase64.net/doku.php?id=base:filter_tips (DEAD)
- https://codebase64.net/doku.php?id=base:hard_restart (DEAD)
- https://codebase64.net/doku.php?id=base:start
- https://codebase64.net/doku.php?id=base:demo_programming (DEAD parent)
- https://codebase64.net/doku.php?id=base:advanced_irq_music_player (DEAD)
- https://codebase64.net/doku.php?id=base:effective_use_of_voice_3 (DEAD)
- https://codebase64.net/doku.php?id=base:filter_and_resonance (DEAD)
