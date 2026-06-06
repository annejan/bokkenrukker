# SID Composition Workflow for GoatTracker

A practical companion to `readme.txt` sections 3.3 - 3.4. The readme documents
what the bytes *mean*; this captures what experienced SID composers actually
*do* with them. All numbers are hex unless noted.

---

## 1. Quick-start workflow for a new SID tune

1. **Pick chip target first.** `Shift+F8` toggles 6581/8580. For "any C64"
   portability, avoid filters and combined waveforms `$30` (tri+saw) and
   `$50` (tri+pulse).
2. **Set tempo first.** `F06` is the Hubbard/Galway rate; `F04` suits fast
   arpeggios; `F03` is fastest sane and forces gate timer `$02` everywhere
   (gate timer must be `tempo - 1`).
3. **Design three instruments before notes**: bass (01), lead (02), drum (03).
4. **Channel split.** Default: ch1 bass, ch2 lead, ch3 drums + harmony.
   Arp-heavy: ch1 bass, ch2 arp lead, ch3 drums stealing ch3 from a held
   chord briefly.
5. **Sketch in jam mode** (space toggles). `F1` plays from start, `F3` loops
   current pattern.
6. **Bass first, then melody on ch2 using arpeggio wavetables only.** No
   portamento or vibrato yet.
7. **Drums on ch3**: kick on each beat, hat off-beat, snare on 2 and 4.
   Absolute-pitch wavetable steps so drums sound the same regardless of
   pattern note.
8. **Fix the instrument the moment you hate it.** Don't pile patterns on a
   broken sound.

---

## 2. Instrument design recipes

Each recipe lists A/D, S/R, plus wavetable / pulse / filter programs.

### Bass (pulse, classic)
A/D `09`, S/R `06`. The "thwack" bass. Held variant: `0A / FA`.
- Wave: `01: 41 00 / 02: FF 01`.
- Pulse: `01: 88 00 / 02: 7F 03 / 03: FF 02` (slow PWM drift). Static pulse
  sounds dead; slow PWM mimics analog detune.

### Bass (triangle, sub)
A/D `08`, S/R `0A`. Wave `01: 11 00 / 02: FF 01`. No pulse, no filter.
Muffled in the low register - good as a sub under a saw lead.

### Lead (sawtooth, filtered)
A/D `08`, S/R `A8`. Wave `01: 21 00 / 02: FF 01`.
- Filter: `01: 90 F1 / 02: 00 40 / 03: 7F 02 / 04: FF 00` - lowpass on ch1
  sweeping up. The filter opening is what gives the lead attack expression.

### Lead (pluck / koto)
A/D `04`, S/R `89`. Snappy attack into low sustain.
- Wave: `01: 41 01 / 02: 40 00 / 03: 41 00 / 04: FF 03` - pulse one
  semitone up for 1 tick, gate off briefly, then pulse on root. The
  "ghost click" is the canonical SID pluck.
- Pulse: `01: 88 00 / 02: FF 00`.

### Pad
A/D `9B`, S/R `2A`. Slow attack, low sustain.
- Wave: `01: 11 00 / 02: 03 80 / 03: 21 80 / 04: FF 01` - alternates
  triangle and saw every 3 ticks while keeping pitch (`80` = pitch
  unchanged). Movement without filter modulation.

### Brass
A/D `08`, S/R `A6`. Wave as filtered lead but pulse `$41`.
- Pulse: `01: 88 00 / 02: 40 20 / 03: 40 E0 / 04: FF 02` - PWM up then down,
  vowel-like sweep.
- Filter: `01: A0 F1 / 02: 00 60 / 03: FF 00` (bandpass, high resonance).

### Organ
A/D `00`, S/R `F0`. Instant on, hold forever.
- Wave: `01: 41 00 / 02: 11 0C / 03: 41 00 / 04: 11 0C / 05: FF 01` -
  alternates pulse root with triangle an octave up (fake additive). Static
  `88 00` pulse, no filter.

### Kick drum
A/D `06`, S/R `00`. Wave: `01: 11 D8 / 02: 11 D2 / 03: 11 CC / 04: 80 C0 /
05: FF 00` - falling triangle pitch on absolute notes, then a flick of low
noise. Identical regardless of pattern note.

### Snare
A/D `08`, S/R `A9`. Readme example expanded: `01: 81 D0 / 02: 41 AA /
03: 41 A4 / 04: 80 D4 / 05: 80 D1 / 06: FF 05` - high noise opening, pulse
"thunk" body, looping noise tail. Pulsewidth `800` recommended.

### Hi-hat closed / open
Closed: A/D `02`, S/R `06`, wave `01: 81 DF / 02: 80 DC / 03: FF 00` (all
noise, highest pitches). Open: same wave, S/R `0A`, extend tail and loop
on the last two steps.

### Hard restart cheat sheet
Default gate timer `$02` fires hard restart 2 ticks before the note - the
"click" defining the SID attack. Bit `$80` (`$82`) disables hard restart
for legato. Bit `$40` (`$42`) disables gateoff for ringing tails. Gate
timer cannot exceed `tempo - 1`.

---

## 3. Wavetable arpeggio cookbook

Wavetable right column: `00-5F` = positive semitones, `60-7F` = negative,
`80` = keep pitch, `81-DF` = absolute notes. Left column `00` = waveform
unchanged, `01-0F` = delay 1-15 frames, `10-FF` = waveform value (with
`FF` reserved for jump).

```
Major triad  (1, 3, 5)         Minor triad  (1, b3, 5)
01: 21 00                      01: 21 00
02: 00 04                      02: 00 03
03: 00 07                      03: 00 07
04: FF 01                      04: FF 01

Sus4 (1, 4, 5)                 Major 7 (1, 3, 5, 7)
01: 21 00                      01: 21 00
02: 00 05                      02: 00 04
03: 00 07                      03: 00 07
04: FF 01                      04: 00 0B
                               05: FF 01

Power chord (1, 5, 8va)        Octave bounce
01: 21 00                      01: 21 00
02: 21 07                      02: 21 0C
03: 21 0C                      03: FF 01
04: FF 01

Diminished (1, b3, b5, 6)      Slow arp (3-frame hold per step)
01: 21 00                      01: 21 00
02: 00 03                      02: 02 04
03: 00 06                      03: 02 07
04: 00 09                      04: FF 01
05: FF 01
```

Design rule: **arp loop length x tempo = chord rhythm**. A 3-step arp at
tempo 6 cycles fast and glassy; a 4-step arp at tempo 4 cycles per row and
sounds like 16th-note hammer-ons.

---

## 4. PWM and filter idioms

### Slow PWM drift
```
01: 88 00           ; pulse $800
02: 7F 03           ; +3/frame for 127 frames
03: 7F FD           ; -3/frame (signed FD = -3)
04: FF 02
```

### Fast PWM buzz
Wide signed values wrap through inaudible `$000`/`$FFF` for a buzzy,
sync-like timbre. Use sparingly.
```
01: 88 00
02: 7F 60
03: FF 02
```

### Hollow / nasal pulse
A pulse near `$080` or `$F80` is hollow and vocal. `01: F8 00 / 02: FF 00`.

### Classic LP filter sweep (Galway/Hubbard lead opener)
```
01: 90 F1           ; LP, resonance F, channel 1
02: 00 10           ; start cutoff $10
03: 7F 02           ; +2/frame
04: FF 00
```

### Wah (up and back)
```
01: A0 87           ; bandpass, resonance 8, all channels
02: 00 00
03: 7F 01
04: 7F 01
05: 7F FF           ; signed -1
06: 7F FF
07: FF 03           ; loop
```

### 6581 vs 8580 gotchas
- **6581 cutoff is non-linear and chip-specific** - the same value sounds
  different on every machine. The "safe" portable range is roughly
  `$30 - $C0`.
- **6581 distorts musically** at high resonance; 8580 stays clean. 6581 is
  warmer and dirtier, 8580 is precise and consistent.
- **Combined waveforms differ**: `$30` (tri+saw) is silent on 6581 but
  audible on 8580. `$50` (tri+pulse) works on both but tonally differs.
- **Ben Daglish strategy**: never use filters, so the tune sounds identical
  on every C64. Conservative and effective.

---

## 5. Multi-channel arrangement patterns

The SID has 3 voices. Channel allocation is the single biggest creative
decision.

- **Chiptune / arp-heavy**: ch1 pulse bass, ch2 saw + arp wavetable lead,
  ch3 drums (which steal the channel for 4-8 ticks per hit, then return
  to silence or pad).
- **Demoscene "in-game" (Hubbard, Galway, Tel)**: ch1 bass with filter
  sweeps on long notes, ch2 pulse lead with PWM and delayed vibrato, ch3
  rhythmic noise+pulse hybrid (Tel's *Sanxion* trick: continuous shaker
  *plus* bass on the same channel).
- **Game music / ambient**: ch1 sparse bass, ch2 detuned pulse pad
  (slow PWM, slow release), ch3 melody. No drums.
- **Three-melody counterpoint (Linus Akesson school)**: each channel plays
  a melodic line on a different waveform (saw + pulse + triangle).
  Rhythm comes from line interplay; no drums.
- **Ring-mod special**: ringmod on ch1 uses ch3 as modulator (and so on
  cyclically). Common idiom: ch3 silent triangle at an inharmonic
  frequency, ch1 triangle with bit `$04` set for bell tones. Cost: ch3 is
  consumed.

---

## 6. GoatTracker workflow tips

- **8XY is for echo/variation, not new notes.** Use it when the *same* note
  switches timbre mid-phrase. New note + new sound = new instrument number.
- **9XY retriggers PWM mid-note.** A held bass goes stale; 9XY to a fresh
  pulse program revives it.
- **AXY chains filter "presets".** Make multiple filter programs and trigger
  them from patterns - avoids duplicate instruments.
- **0XY clears a stuck realtime command** (1/2/3/4) without changing
  anything else.
- **EXY funktempo** is the only way to swing - two tempos like `09 06`
  give a 16th-note shuffle.
- **6XY mid-note dropping sustain to 0** gives an "echo cutoff". Combine
  with long release for natural decays.
- **Instrument 63's A/D doubles as song default tempo** if unused otherwise.
- **`Shift+RETURN` in a vibrato/portamento field** auto-creates a speedtable
  entry from the old-style raw value.

### Common workflow pains
- One instrument = 5-15 min of "edit ADSR - play - hate it - edit wavetable
  - retune pulse - okay". The slow part is diagnosis: *is the dullness in
  attack, pulse width, or lack of filter?*
- Cloning an instrument means re-entering ADSR by hand and remembering its
  table references to avoid mutating the original.
- Spotting two patterns that differ only by transpose is hard.
- A new drum kit = 3-5 hand-crafted instruments.

---

## 7. UI implications for the Qt frontend

### InstrumentView
- **Template `QComboBox`** above the spinbox grid: Bass-pulse, Bass-tri,
  Lead-saw, Lead-filtered, Pluck, Pad, Brass, Organ, Kick, Snare,
  HatClosed, HatOpen. Selecting writes the A/D, S/R, table pointers, and
  pre-fills the referenced wave/pulse/filter slots from section 2.
- **"Clone instrument" `QPushButton`** that deep-copies the 9 params *and*
  the referenced table programs into fresh slots, so editing the clone
  never mutates the original.
- **"Stop pulse" `QCheckBox`** as sugar for the readme tip: auto-inserts
  `80 00 / FF 00` in a free pulse slot and points the instrument there.
- **ADSR preview** (already exists): overlay a vertical line at the
  gate-timer tick so hard-restart-vs-release is visible.
- **Hard-restart decoded next to gate timer**: tooltip flags whether the
  value exceeds `tempo - 1` and whether bits `$80` / `$40` are set
  ("hard restart off", "gateoff off"). Right now those bits are invisible.

### TablesView
- **Wavetable: "Detect arp" button.** Classify selected right-column
  intervals: `00 03 07` -> minor, `00 04 07` -> major, `00 05 07` -> sus4,
  `00 04 08` -> aug, `00 03 06` -> dim, `00 0C` -> octave bounce. Label
  goes in the legend strip.
- **Wavetable: "Arp cookbook" `QComboBox`** in the legend - selecting a
  chord type writes rows at the cursor (direct impl of section 3).
- **Pulsetable: "PWM sweep wizard" `QDialog`** with QSpinBoxes for start
  width, end width, frames, plus a one-shot/loop QCheckBox. Generates the
  `8X YY / 7F SS / FF XX` sequence. Collapses signed-byte editing into
  intent-level entry.
- **Filtertable: "Filter wizard" `QDialog`** with QComboBox for type
  (LP/BP/HP), QSpinBox for resonance, per-channel QCheckBox, QSpinBoxes
  for cutoff start/end and frames. Filter encoding is the most opaque part
  of the format.
- **Cursor decode footer** (already exists): add a plain-English "what
  this step does" line, e.g. `81 D0` -> "Noise, gate on, absolute G#6".
- **Signed value display**: pulse/filter modulation right-column bytes are
  signed - show both hex and signed decimal (`FD` -> `-3`).

### PatternView
- **Per-command `QToolTip`** on the command column - hover `8` -> "set
  wavetable pointer", `9` -> "set pulsetable pointer", etc.
- **Highlight duplicate patterns** in the order list with an R-command
  reuse hint. Biggest single win for tune compression.
- **"Transpose pattern" `QAction`** with preview - much faster than
  retyping notes.
- **Drum kit picker `QComboBox`** with presets ("Standard", "808") that
  populate instruments 03-05 from section 2.

### Cross-cutting
- **Status-bar `QLabel` showing 6581/8580** with one-click toggle. Chip
  target affects everything; making it ambient prevents silently writing
  6581 filter values into an 8580 song.
- **Tempo / gate-timer consistency `QLabel`**: non-modal warning if any
  instrument's gate timer >= tempo. Cheap, prevents silent playback bugs.

---

## Sources

- `/home/paul/work/c64/goattracker2/readme.txt` (GoatTracker v2.72 manual,
  sections 3.2 - 3.6 - primary source for all format details).
- [GoatTracker tutorial PDF, zerozillion (csdb.dk)](https://csdb.dk/getinternalfile.php/77611/goattracker_tutorial.pdf) - step-by-step on ADSR,
  wavetable, pulse, filter, arpeggios. Fetched OK as PDF, converted with
  `pdftotext`.
- [Cadaver - Building a musicroutine](https://cadaver.github.io/rants/music.html) -
  hard restart methods, channel processing idioms.
- [Chordian.net - Composing in SID Factory II part 4 (Instruments)](https://blog.chordian.net/2022/08/27/composing-in-sid-factory-ii-part-4-instruments/) -
  ADSR shapes (`00 a0`, `9b 2a`, `04 69`), pad and pluck recipes; SF2
  tables are conceptually identical to GoatTracker's.
- [Battle of the Bits - GoatTracker Effects Commands](https://battleofthebits.com/lyceum/View/GoatTracker+Effects+Commands) - command quick reference.
- [MusicRadar - SID-style bass sound](https://www.musicradar.com/how-to/how-to-create-a-sid-chip-style-bass-sound) - filter cutoff / resonance heuristics.
- [The Conversation - 35 years of SID](https://theconversation.com/the-sound-of-sid-35-years-of-chiptunes-influence-on-electronic-music-74935) - Hubbard/Galway/Tel
  idioms (the Sanxion ch3 noise+pulse trick).
- [Linus Akesson - Elements of Chip Music](http://www.linusakesson.net/music/elements/) - polyphony-via-arpeggio,
  three-melody arrangement.
- [Cadaver - Covert BitOps tools](https://cadaver.github.io/tools.html) - GoatTracker project home.
- Dead/blocked at fetch time but available via search summary:
  [Retro64 part 1](https://retro64.altervista.org/blog/making-commodore-64-music-the-sid-and-goattracker/),
  [Retro64 part 2](https://retro64.altervista.org/blog/making-commodore-64-music-with-goattracker-part-2/),
  [chipmusic.org typical SID sounds](https://chipmusic.org/forums/topic/16010/what-are-the-typical-c64sid-soundsinstruments/),
  [chipmusic.org goattracker tips](https://chipmusic.org/forums/topic/14953/goattracker-tips/),
  [chipmusic.org 6581 vs 8580](https://chipmusic.org/forums/topic/17495/c64-sid-shootout-6581-vs-8580/),
  [angelfire chiptune-arpeggio tutorial](https://www.angelfire.com/music4/b1itz_lunar/chiptune.htm) (ECONNREFUSED, link dead).
