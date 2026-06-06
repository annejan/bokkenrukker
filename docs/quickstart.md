# GoatTracker Qt — Quickstart for tracker veterans

This guide assumes you've used a tracker before (ProTracker, OpenMPT, Renoise,
Schism, FastTracker 2…) but never written a SID tune. It targets the new Qt6
frontend in `qt/`. The underlying engine, file format and command alphabet are
identical to the canonical SDL build by Lasse Öörni, so any community resource
about GoatTracker 2.x applies. Where this guide says "GoatTracker" without
qualification, it means version 2.72 and later.

## If you know ProTracker, here's what's different

* **There are no samples.** The SID chip is a 3-voice subtractive synth. Every
  instrument is a *program* that drives three oscillators (triangle / sawtooth /
  pulse / noise) plus a shared analog filter. You sculpt sound, you don't load
  it.
* **Three channels, not four.** And channels are *not* interchangeable:
  ring-mod and sync on channel 1 read from channel 3, channel 2 reads from
  channel 1, channel 3 reads from channel 2. Putting your bass on channel 2 vs
  channel 3 actually matters.
* **One global filter.** Only one channel at a time should host the
  filter-controlling instrument. Multiple channels driving the filter table
  will fight each other.
* **Notation is hex, everywhere.** Pattern numbers, instrument numbers, ADSR
  nybbles, pulse width, table positions — all hex `$00`..`$FF`. The Qt
  instrument editor prefixes hex spinboxes with `$`; pattern data and tables
  are displayed without the `$`.
* **Effects are different.** `1XY` and `2XY` are portamento up/down like
  ProTracker, but `XY` is an *index into the speed table*, not the slide rate
  itself. `3XY` is tone-portamento (matches ProTracker), `4XY` is vibrato (also
  speed-table indexed), `FXY` is set-tempo. There is no Axx volume-slide:
  envelopes do the work.
* **Instruments don't carry their full sound.** An instrument is just a thin
  record (ADSR + 4 table pointers + a few timing bytes). The *actual*
  waveform changes live in the wavetable, pulse-width changes live in the
  pulse table, filter changes in the filter table.

## Your first tune in 5 minutes

1. **Launch the Qt build:** `qt/build/goattracker-qt` (built via
   `cd qt && cmake -B build && cmake --build build`).
2. **Press F7** to enter the instrument editor. Pick instrument `$01`, type a
   name. Set Attack `0`, Decay `9`, Sustain `0`, Release `0`. Set *Wavetable
   Pos* to `$01`. (Default pulse/filter/vibrato pointers `$00` are fine —
   `$00` means "don't touch".)
3. **Press F8** to enter the tables editor. The cursor is on Wave table step
   `01`. Type `41 00`, ENTER → `FF 00`. That's a pulse-waveform tone that
   stops after one step. (Left byte `41` = pulse + gate; right byte `00` =
   "relative note +0", i.e. play the note you triggered.)
4. **Press F5** to enter the pattern editor. The keyboard is laid out like
   ProTracker by default: `Z S X D C V…` plays an octave. Hit space to toggle
   record, play some notes — they appear with instrument `$01` next to them.
5. **Press F6** to see the order list. By default channel 1's order list is
   just pattern `00` then `RST 00` (restart to step 0). Add patterns by
   pressing `+`/`-` to bump the value, or just leave it as is.
6. **Press F1** to play from the beginning. **F4** stops.

That's the entire loop. Pattern → Order → Instrument → Tables → Pattern.

## Understanding the four tables

The four tables (Wave, Pulse, Filter, Speed) are the *single most
important thing to internalize*. Each table is a 255-step list of byte pairs
(left byte L, right byte R) interpreted as a tiny program. Your instrument
holds a pointer into each table; on every tick the player advances that
pointer and runs the cell.

| Table  | What it controls                                  | Jump (FF) |
|--------|---------------------------------------------------|-----------|
| Wave   | Waveform register + relative/absolute note offset | yes       |
| Pulse  | Pulse-width register (8XX… set, 1–7F modulate)    | yes       |
| Filter | Cutoff, resonance, passband, channel mask         | yes       |
| Speed  | Vibrato/portamento/funktempo parameter bank       | no jumps  |

The Wave table is where arpeggios, drum noise sweeps, ring/sync hits and
multi-stage envelopes live. The Speed table is the most confusing for
newcomers — see "gotchas" below.

The semantic load on the *left byte* of the Wave table is enormous, and the
Qt tables view has a per-table legend pinned above the grid plus a live
decode of the cursor cell at the bottom — use them.

## Common gotchas

* **Attack 0 + first wavetable step is silent.** The SID needs a frame to
  start an envelope. If `A=0` and your wavetable starts on a real waveform,
  the first row plays nothing. Either bump `A` to `1`, or add a delay/test
  step (`09 00` or `E9 00`) at the very top of the wavetable.
* **Wave-table left byte zero ("no change") is illegal on step 1.** It must
  appear *after* a real waveform was set. The editor will let you do it; the
  player will eat your note.
* **`FF` jump position is 1-based.** `FF 02` jumps to step `02`, not `01`.
  Off-by-one is the most common wavetable bug.
* **Never jump *onto* a jump.** A wavetable, pulse table or filter table
  `FF` followed by another `FF` (or jumped to via `8XY`/`9XY`/`AXY`) is
  undefined behavior. The Qt editor does not currently warn.
* **Pulse/Filter pointer `$00` in instruments means "leave running".** Set
  it to a real step number to (re)start that table; use pattern command
  `900`/`A00`/`B00` to stop them.
* **Speed-table indices need entries.** `4XY` vibrato, `1XY/2XY/3XY`
  portamento and `EXY` funktempo all use `XY` as a speed-table pointer.
  Index `00` is "no effect" but valid for `3XY` (tie-note). You can paste a
  ProTracker-style raw vibrato/portamento value into the parameter and hit
  `SHIFT+RETURN` to auto-create a matching speedtable entry.
* **Order list endmark is `RST <pos>`.** The byte before `RST` must be a
  pattern number, never a TRANSPOSE or REPEAT command. Editor will halt
  playback if you violate this.
* **Save as `.sng`, always.** `.prg`, `.bin`, `.sid` exports cannot be
  loaded back — they are packed/relocated output.
* **6581 vs 8580.** Compose toward one. Filter behaviour, pulse-width
  bleed and combined waveforms differ between the two chip revisions; the
  toolbar SID model toggle (SHIFT+F8) swaps them at runtime so you can
  audition. Modern compositions usually target 8580; SidWizard or classic
  Hubbard/Galway aesthetics need 6581.
* **Hex isn't optional.** You'll be fluent in `0`–`F` within an hour. A
  decimal calculator on a second monitor is fine for the first day.

## Where to learn more

* **`readme.txt`** in the project root — the 1900-line canonical reference.
  Sections 2.3 (keyboard) and 3 (song data) are the meat.
* **`goat_tracker_commands.pdf`** in the project root — Simon Bennett's
  one-page quick-reference for all pattern commands.
* **Hein Holt's "Ravenspiral" GoatTracker command chart** — pocketable PDF
  cheat sheet, mirrored on multiple chiptune sites.
* **Retro64 tutorial series** — beginner-friendly multi-part walkthrough
  including a Rob Hubbard "Delta" remake:
  `https://retro64.altervista.org/blog/making-commodore-64-music-the-sid-and-goattracker/`
* **Nordisch Sound's video tutorials on YouTube** — search "GoatTracker
  SID Tutorial Unleashed", "Goat Tracker Starting Tutorial wave forms pulse
  modulation".
* **ChipMusic.org forum, Commodore Computers board** — the threads
  "Goattracker tips?" and "Can someone teach me the arcane ways of
  goattracker?" capture years of community Q&A.
* **csdb.dk** has the "Basic guide for creating SID music with GoatTracker
  v2.67" PDF by ne7 — still mostly current.
* **`examples/` directory** — open every `.sng` in there and read its
  instrument/table layout. See `examples-tour.md` for what each one
  demonstrates.

When you get stuck: open one of `cabrinigreen.sng`, `sanction.sng` or
`dojo.sng`, jump to F7/F8 and study how the instrument tables are wired.
Reading SID tunes is the fastest path to writing them.
