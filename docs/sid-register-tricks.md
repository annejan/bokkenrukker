# SID Register Tricks — A Composer-Oriented Field Guide

Idiomatic patterns SID composers (Hubbard, Galway, Hülsbeck, Tel, JCH,
Akesson, …) commit to bytes, and how the Qt goattracker frontend in
`/home/paul/work/c64/goattracker2/qt/` can surface them as first-class
affordances rather than raw hex.

---

## 1. Register cheat sheet ($D400-$D418)

The SID has 25 write registers. The pattern repeats 3× for the voices,
then a small filter block.

| Reg     | Name        | Bits 7..0                                        |
|---------|-------------|--------------------------------------------------|
| $D400   | V1 FREQ LO  | 16-bit oscillator phase increment (lo)           |
| $D401   | V1 FREQ HI  | (hi)                                             |
| $D402   | V1 PW LO    | 12-bit pulse width (lo 8 bits)                   |
| $D403   | V1 PW HI    | (top 4 bits in nybble 0-3; nybble 4-7 unused)    |
| $D404   | V1 CTRL     | **NOIS PULS SAW  TRI  TEST RING SYNC GATE**      |
| $D405   | V1 AD       | Attack (hi-nyb) / Decay (lo-nyb)                 |
| $D406   | V1 SR       | Sustain (hi-nyb) / Release (lo-nyb)              |
| $D407-$D40D | V2 …    | identical layout                                 |
| $D40E-$D414 | V3 …    | identical layout                                 |
| $D415   | FC LO       | 11-bit filter cutoff (only lo 3 bits used)       |
| $D416   | FC HI       | (top 8 bits — most of the resolution lives here) |
| $D417   | RES/FILT    | RES3 RES2 RES1 RES0 EXT F3 F2 F1                 |
| $D418   | MODE/VOL    | 3OFF HP   BP   LP   VOL3 VOL2 VOL1 VOL0          |

`$D41B` (OSC3) and `$D41C` (ENV3) are read-only and historically used by
games to drive visuals from voice 3.

**CTRL register pairings for SYNC/RING** (goattracker readme §3.4.1):
voice 1 is modulated by voice 3, voice 2 by voice 1, voice 3 by voice 2.
Ring mod requires the triangle waveform on the *carrier* voice.

---

## 2. Waveform combo gallery

Single waveforms (`$11` tri, `$21` saw, `$41` pulse, `$81` noise) are
reliable. Combinations exist because the waveform selectors share a DAC
bus; the original spec implied a logical AND but the real mix is
closer to "weakest-wins" with chip-dependent bit leakage.

| Hex   | Combo            | 6581                           | 8580                          | Used for                       |
|-------|------------------|--------------------------------|-------------------------------|--------------------------------|
| $31   | tri + saw        | very thin, almost silent       | nasal buzz, audible           | lead "sting"                   |
| $51   | tri + pulse      | hollow flute / wet clarinet    | clearer, breathy              | Hubbard *Knucklebusters* leads |
| $61   | saw + pulse      | reedy organ, dirty             | mellow organ, cleaner         | Galway-style organ             |
| $71   | tri + saw + pulse| paper-thin, chip-dependent     | slightly fuller               | one-shot stab transients       |
| $81+x | noise + anything | **locks the LFSR**             | locks the LFSR                | recover via `$08` test bit     |

Two rules every composer internalises:

1. **Noise never combines.** `$C1`, `$A1`, `$91` silence the LFSR and
   hang the channel after a couple frames. Recovery: one frame of
   `$08` (test), then `$81` again.
2. **Combined waves drift between chips.** What sounds rich on a
   6581R4AR is razor-thin on an 8580R5. JCH and Reyn Ouwehand wrote
   leads as `$41` plus a PWM table — identical on both chips.

Plogue's waveform captures showed two playbacks of the same combined
waveform on the same chip differ by a few bits — never expect
bit-exact reproducibility from combos.

---

## 3. Filter playbook

Cutoff register is 11-bit ($000-$7FF) but goattracker only exposes the
top 8 bits via the filter table's right-side byte. Resonance is a 4-bit
nybble in `$D417` hi. The lo nybble of `$D417` is a channel bitmask:
bit 0 = V1, bit 1 = V2, bit 2 = V3, bit 3 = external in.

Practical recipes (cutoff value is the right-side of a `00 XX` step,
mode/res is the right-side of an `8X-FX` step):

| Effect              | Mode   | Cutoff  | Res | Mask | Notes                                   |
|---------------------|--------|---------|-----|------|-----------------------------------------|
| Classic lead sweep  | LP $90 | $20→$F0 | $F  | $1   | Galway organ; modulate cutoff 1/frame   |
| Vocal "wow" phaser  | BP $A0 | $40→$A0 | $8  | $7   | bandpass swept slowly over all 3 voices |
| Bass thump          | LP $90 | $30     | $4  | $1   | static cutoff, low res; punchy 8580 kick|
| Hi-hat sizzle       | HP $C0 | $D0     | $0  | $4   | high-pass voice 3 noise, no res         |
| Dub delay tail      | LP $90 | $60→$10 | $C  | $1   | slow descending sweep, high res         |
| Hubbard-style bell  | BP $A0 | $C0     | $F  | $1   | resonant peak rings the ring-mod        |

The 6581 cutoff curve is famously non-linear and varies wildly between
chips. JCH's advice: write tables that *modulate* cutoff rather than
rely on absolute values, and run filter-using instruments on one voice
at a time. A canonical goattracker filter table (slow bandpass sweep,
looped):

```
01: A0 87   bandpass, res=8, mask=all
02: 00 00   cutoff = 0
03: 7F 01   +1 for 127 ticks
04: 7F 01   continue (saturates near top)
05: 7F FF   -1 for 127 ticks
06: 7F FF   continue
07: FF 03   loop to step 03
```

---

## 4. Ring-mod / sync recipes

Both effects live in CTRL bits 2 (ring) and 1 (sync). Channel pairing is
fixed in silicon (V1←V3, V2←V1, V3←V2). The carrier needs the triangle
bit set (`$10`) for ring-mod to be audible.

**Tubular bell** (Hubbard, *Last Ninja 2 — "Central Park"*, 1988; ring
mod V1 by V3). Carrier wavetable: `15 00` (tri+ring+gate) sustained
with `FF 01`. A separate V3 instrument plays the modulator (often a
fifth above — `+07` in the wavetable R column).

**Metallic gong** (Galway, *Wizball*, 1987). Carrier holds `15 00`;
modulator on V3 slides `11 00 → 03 02 → 03 02 → FF 01` (tri+gate then
two `+2` frequency steps).

**Robot voice** (Tel, *Cybernoid II*, 1988). V2 `23 00` (saw+sync+gate)
with V1 playing a sub-audio frequency — use `$E1` "inaudible $01" to
sustain V1 silently while still driving V2's sync.

**Snare with ring rattle** anchors timbre with absolute-note R bytes:

```
01: 81 D0   noise, abs note D0
02: 15 AA   tri+ring, abs note AA
03: 41 A4   pulse, abs note A4
04: 80 D4   silent noise hold
05: FF 00
```

---

## 5. ADSR / gate-bit tricks

The infamous **ADSR bug**: when the envelope counter is reused without
a clean reset, a note with `A=0, R=1` can delay its attack by up to 32
seconds because the internal counter must roll all the way around.
Composers cope with **hard restart**: forcing gate off and writing a
specific ADSR for 2-3 frames before gating the new note.

Goattracker's `-Axx` (or `SHIFT+F7`) sets the hard-restart ADSR.
Documented presets:

| Value  | Character             | Author / scene origin              |
|--------|-----------------------|------------------------------------|
| $0000  | hardest possible      | only useful with gateoff timer 1   |
| $0F00  | **default**, soft     | goattracker / SF2 default          |
| $0F01  | soft with tiny release| smoother for slow legato passages  |
| $000F  | pronounced note onset | Galway-style sharp re-trigger      |
| $FF00  | aggressive            | "Jeff"-style harsh re-articulation |
| $0FF0  | balanced              | Randall / Jammer                   |
| $0F20  | mid-soft              | GRG                                |

**Attack-F** ($Fxxx HR ADSR) flips the playroutine to write waveform
*before* ADSR (readme 255-258), improving reliability for ultra-fast
`R=0/1` releases. Surface this as "alternate write order" rather than
a magic number.

The **1st-frame waveform** is normally `$09` (gate + test) to lock
oscillator phase. Specials: `$00` leaves waveform and gate alone,
`$FE` forces gate off, `$FF` forces gate on. The **legato bit `$40`**
in the gate timer suppresses both gateoff and hard restart — Hubbard's
lyrical bass lines lean on this constantly.

**Multi-trigger drum** via test bit inside the wavetable:

```
01: 81 D0   noise, abs D0     ; first hit
02: 09 00   test+gate         ; restart oscillator, silent
03: 81 D0   noise again       ; second hit
04: FF 00
```

**"Ping" envelope**: A=0, D=9, S=0, R=0 — full volume instantly, fade
to silence in ~90 ms.

---

## 6. Reference tunes

| Technique                 | Tune                              | Author    | Year |
|---------------------------|-----------------------------------|-----------|------|
| Saw+pulse organ           | *Monty on the Run* main theme     | Hubbard   | 1985 |
| Ring-mod bells            | *Last Ninja 2 — Central Park*     | Hubbard   | 1988 |
| Sync robot voice          | *Cybernoid II*                    | Tel       | 1988 |
| Fast PWM lead             | *Wizball*                         | Galway    | 1987 |
| Arpeggio fake chord       | *Comic Bakery*                    | Hubbard   | 1985 |
| Filter sweep + resonance  | *Crazy Comets*                    | Hubbard   | 1985 |
| Combined waveform tricks  | *International Karate*            | Hubbard   | 1986 |
| HP/BP filter on noise     | *Spellbound* hihats               | Galway    | 1985 |
| Sampled drums via $D418   | *Arkanoid*, *Game Over*           | Galway    | 1987 |
| Ring-mod gong             | *Bionic Commandos*                | Hülsbeck  | 1988 |
| Three-voice filter routing| *A Mind Is Born*                  | Akesson   | 2017 |
| 8x multispeed analogue    | *Cybernoid II* lead               | Tel       | 1988 |

---

## 7. UI affordances for the Qt frontend

Concrete proposals. Each names the file in `qt/`, the widget, the data
source, and the user payoff.

### 7.1 Waveform bit selector (`TablesView.cpp`, wave table footer)

When the cursor lands on a wave-table row in `$10-$DF`, expand the
decode footer with an **8-checkbox bitfield editor** for the left byte
(noise/pulse/saw/tri/test/ring/sync/gate). Re-use the colour tints
already in `WavetablePreview::paintEvent` so the checkboxes mirror the
preview swatches. `QCheckBox::toggled` rewrites `ltable[WTBL][etpos]`
and calls `update()`.

### 7.2 6581/8580 warning badge (`TablesView.cpp`)

Add an `enum class TargetChip { SID6581, SID8580 }` to a new small
header (e.g. `qt/SidChip.h`). Default from a `QSettings` key. When the
cursor cell decodes to a *combined* waveform, paint a 14px round badge
to the right of the decode line:

* 6581 + `$31` (tri+saw): amber "thin on 6581" tooltip
* 6581 + `$71`: red "near-silent on 6581"
* 8580 + `$51`: cyan "louder than 6581"
* noise + any other bit: red "LFSR will lock — needs $08 reset"

Drive it from a `static const struct { uchar mask; ChipFlags } combos[]`
table near the existing `tableName[]` array.

### 7.3 Filter cutoff slider (`TablesView.cpp`, filter table footer)

For filter-table `00 XX` rows, draw a horizontal `QSlider` (0-255)
below the decode line, with a log frequency scale tick-marked at
100 Hz / 1 kHz / 5 kHz. Slider colour = `tableTint[2]`. Mutates
`rtable[FTBL][etpos]` live. A 4-LED strip (V1/V2/V3/ext) visualises
the channel bitmask of the most recent `8X-FX` row above; click an LED
to flip its bit. Active LEDs use `Theme::C::vuGreen`.

### 7.4 Ring-mod / sync indicator (`PatternView.cpp` header)

Above the three voice columns, paint a 12px-high "modulation matrix"
strip: three arcs joining V1↔V3, V2↔V1, V3↔V2. Highlight an arc in
`Theme::C::highlight` when *any* row currently playing has CTRL bits 1
or 2 set on the source voice. Tooltip names the carrier and modulator.
This makes ring-mod and sync visible at the pattern level rather than
buried in the wave-table; players currently have to read hex.

### 7.5 PWM range guide (`TablesView.cpp`, pulse table)

For pulse-table `8X-FX` rows, render a horizontal bar under the decode
line representing the 12-bit pulse width `$000-$FFF` with a caret at
the current value, plus reference ticks at `$080` (narrow), `$800`
(square), `$F80` (inverse-narrow). For `01-7F` modulation rows, draw
two carets (start + end of sweep) and an arrow showing direction and
per-tick speed. New helper class `PwmBar : QWidget`.

### 7.6 Hard-restart pattern picker (`InstrumentView.cpp`, Misc box)

Replace the bare `gateTimer_` hex spinbox with a row containing:

1. The existing spinbox.
2. Two checkboxes "Legato ($40)" and "No HR ($80)" XORing those bits.
3. A `QComboBox` "HR preset" with the seven §5 entries (0F00 Soft
   default, 000F Pronounced, 0F01 Soft+release, FF00 Aggressive,
   0FF0 Balanced, 0F20 Mid-soft, 0000 Hardest).

Selection writes the global hard-restart ADSR via a new exported C
symbol (currently set only by `-Axx`).

### 7.7 ADSR envelope preview overlays (`InstrumentView.cpp`)

Augment `AdsrPreview::paintEvent`: draw a thin red dashed line when
`A=0, R=1` labelled "ADSR-bug risk — set HR to FFxx"; label `S=0, R=0`
as "Ping"; show a clock icon when `A>=12` warning that multispeed
articulation may suffer.

### 7.8 Wavetable colour-key legend (`TablesView.cpp`)

Beneath `WavetablePreview`, add a one-line strip of six 10×10 swatches
labelled "noise / pulse / saw / tri / delay / cmd", reusing the
existing `QColor` constants.

### 7.9 Reference-tune popover (`InstrumentView.cpp`)

`QPushButton` "Insert example…" next to the instrument name opens a
`QDialog` with a `QListWidget` of recipes (Hubbard bell, sync robot,
PWM lead, snare, kick, hi-hat). Selecting one writes a hard-coded
`std::vector<uchar>` into the wavetable at the next free slot and
points `instr[einum].ptr[WTBL]` there. Source data: snippets in §4-§5.

### 7.10 Multispeed indicator (`StatusStrip.cpp`)

A small chip in the status strip showing the current multispeed factor
(1x/2x/3x/4x/8x) from the song header. Tooltip cites readme §3.7 and
explains why fast pulse sweeps stutter at 1x.

---

## Sources

- goattracker2 `readme.txt` §2.1–§3.7 (local, the most concrete source)
- *Composing in SID Factory II, Pt 4 — Instruments* (blog.chordian.net)
- CSDB forum thread *Sid ADSR problem / hardrestart* (csdb.dk)
- ChipMusic forum *GoatTracker tips?* (chipmusic.org/forums/topic/14953)
- C64-Wiki SID register reference (c64-wiki.com/wiki/SID)
- oxyron.de SID register table (oxyron.de/html/registers_sid.html)
- Plogue Chipsounds — SID waveform captures (2010)
- INSIDIOUS 6581 reverse-engineering parts 2-4 (insidious6581.blogspot.com)
- Recollection — *The Brief History of SID* (atlantis-prophecy.org)
- jamesm.blog — *The SID Chip: Engineering the Most Iconic Sound*
- Game Journal — *Driving the SID chip* (gamejournal.it)
- Linus Åkesson — `linusakesson.net/music/withering-bytes/` and
  *A Mind Is Born* notes
- MOS 6581 datasheet (6502.org/documents/datasheets/mos/mos_6581_sid.pdf)
- HVSC / DeepSID for tune attributions (deepsid.chordian.net)
