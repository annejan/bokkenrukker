# Examples tour — `examples/*.sng`

A guided read of the example songs that ship in `examples/`. Open each in
the Qt frontend (Ctrl+O), then tab through Pattern (F5), Order (F6),
Instrument (F7) and Tables (F8) to see how the technique is wired. The
titles, authors and command-line arguments come from the `.sng` headers
and from `examples/goatcompo.txt`.

## cabrinigreen.sng — "I'll Be A Pimp In Cabrini Green (When I Grow Up)"

By Randall. The flagship "GoatCompo" entry, intended to be packed with
`/e1 /s1 /a0ff0` (sound-effect channel 1, single subtune, hardrestart
$0ff0). Demonstrates a full-length funky tune driven primarily by
*instrument programming*, not pattern-command spam — the kind of song
where 80% of the magic is in the wave/pulse/filter tables. Pay attention
to how the lead and bass share the order list, how the drum kit fits in
three voices via short noise→pulse wave programs, and to how transpose in
the order list is used to recycle a few patterns across many key changes.

## consultant.sng — "The Consultant"

By Cadaver, 2002 Covert Bitops. A compact, melodic tune from the
prolific Covert Bitops songbook. Good study material for clean
instrument design: short ADSRs, single-line wavetable arpeggios, a
disciplined pulsetable. Note how Cadaver uses very few pattern commands
and lets the wavetable do almost all the work — exactly the
"step-programming" workflow recommended in `readme.txt §3.6`.

## dojo.sng — "Dojo"

By Cadaver, 2002 Covert Bitops. Larger arrangement than `consultant.sng`,
with more voicing tricks. Look here for **ring-modulation usage**: any
instrument with a wavetable left-byte that has bit `$04` set is ring-mod
against the previous channel's oscillator. Also a good example of
**filter sweeps living in the filter table** rather than driven by CXY
cutoff commands — open F8, scroll to the filter table, look for `01–7F`
modulation steps.

## everlasting.sng — "Everlasting Annoyance"

By Richard Bayliss (The New Dimension). Compo entry, packed `/e0 /s1
/a0f00`. Bayliss's style is a useful counterpoint to Cadaver's — more
classic SID pop with conspicuous melodic vibrato and chordal arpeggios
in the wavetable. Excellent example of **four-note arpeggio
wavetables** (the `41 00 / 00 04 / 00 07 / 00 0C / FF 02` pattern from
the readme is directly in here). Learners should compare its
instrument-vibrato delay/param values with `cabrinigreen.sng`'s.

## hyperspace.sng — "My Own Hyperspace"

By Jammer (:P, 2005). Packed `/e1 /s1 /a0ff0`. A demonstration of
**dense effect work** — Jammer was known for writing very textural SID
that uses combined waveforms and unconventional pulse modulation. Read
the pulse table carefully: short modulation steps with rapid sign
changes give the song its restless shimmer. Also instructive for
seeing how multi-channel filter writes are sequenced so they don't
collide.

## sanction.sng — "On A Sanction From CIA"

By Cadaver, Covert Bitops 2005. Compo entry, `/e0 /s1 /a0f00`. Cadaver's
mid-2000s style: hard-edged industrial/rave SID. Demonstrates **driving
basslines via testbit-reset waveforms** (look for `08 00` or `09 00`
wavetable steps acting as percussive note re-attacks), and **funktempo**
in places to push the groove. Open the speed table (F8 → Speed) to see
funktempo entries.

## sixpack.sng — "Sixpack Of Cola"

By No-XS, covered by beek (originally beek's tune). Compo entry. Good
"normal pop SID" reference — clean separation of lead, bass and drum
roles across the three channels, with the drum machine on channel 3.
For beginners: open Instrument $01–$05 and read the wavetable
straight-through; almost every drum hit is encoded as 3–5 absolute-note
wavetable steps and that's the whole drum machine.

## ghosttrackers.sng — "GhostTrackers"

By Hein Holt. Compo entry. Hein is one of GoatTracker's most adventurous
composers; this tune is densely arranged for an 8-bit chip. Pay
attention to how the **order list reuses patterns with TRANSPOSE** to
generate variations without duplicating data. Also a good example of
how SHIFT+L's "convert limit→time" output looks for older modulation
data ported into the current speedtable model.

## funktest.sng — "Covert Ops in 2D (funktempo)"

By Cadaver, 2002 Covert Bitops. A *targeted demo* of the EXY funktempo
command. The whole song's rhythmic feel comes from a single
funktempo speed-table entry alternating two tempo values on
even/odd rows. The shortest path to understanding funktempo: open this
song, press F8, switch to the Speed table, find the row referenced by
the `EXY` commands in the pattern, then watch the LR pair flip the
groove.

## tempo2test.sng — "Tempo2 test (switch POpt off!)"

By Cadaver, 2006 Covert Bitops. The header literally says "switch POpt
off" — toggle the **PO** indicator in the status strip (or run with
`/O0`) before playing or it will glitch. Demonstrates **tempo 2 via
funktempo workaround** (`02 02` in the speed table + gateoff timer 1 in
all instruments, as described in `readme.txt §3.6`). One of those
"actually impossible without a trick" curiosities.

## 2xtest.sng — "MW Title Remix, 2x-speed"

By Cadaver, 10/2001. Multispeed demonstration: the song runs at 2x
player rate (two player ticks per video frame). Useful to compare
against `tempo2test.sng` — different speed-control axes (multiplier vs
funktempo). Note the doubled gateoff timer / HR timer values in the
instruments; multispeed scales them.

## wavecmdtest.sng — "Wavetable command test"

By Cadaver, 2006 Covert Bitops. A focused unit test for the **wavetable
`F0–FE` "execute pattern command" feature** (readme §3.4.1). Open the
wave table (F8 → Wave) and look for `F5`, `F6`, `F7`, etc. cells —
these run pattern commands `5XY`, `6XY`, `7XY` from inside the
instrument's step program. This is how you bake an ADSR sweep or a
mid-note waveform change into an instrument without touching the
pattern.

## transylvanian.sng — "Transylvanian Whipping"

By Cadaver, 2005 Covert Bitops. Longer, moodier tune. Demonstrates
**dynamic ADSR via wavetable F5/F6 commands** — listen for the way lead
notes evolve mid-sustain. Also a good place to see how a single
instrument can host a quite long wavetable "program" (a dozen+ steps)
that resembles a mini synth patch script.

## unleash.sng — "Unleash The F****** Fury"

By Cadaver, 2004 Covert Bitops. The largest example in the bundle.
Full-arrangement metal/rock SID, useful as an integrated study: heavy
use of filter modulation on a single dedicated channel, multiple
subtunes if present, repeated patterns with transpose for verse/chorus
structure. Open the order list (F6) and step through to see the
RST loop point and how Cadaver structures large pieces.

---

## How to use this tour

1. Open a song with **Ctrl+O**.
2. Press **F5** and play (**F1**) to listen.
3. Pause (**F4**), press **F6**, and read the order list. Note how
   many distinct patterns there are vs how often they repeat.
4. Press **F7**, scroll through the instrument list, and pick one that
   sounds interesting in the song.
5. Click the **→ table** button next to its Wavetable Pos. You're now
   reading the actual sound program. Compare it to the wavetable
   examples in `readme.txt §3.4.1`.
6. Once you've decoded one instrument by hand, you can read any SID.

## Sound effect files (`*.ins`)

The directory also ships `sfx_arp1.ins`, `sfx_arp2.ins`, `sfx_expl.ins`,
`sfx_gun.ins`. These are single-instrument `.ins` files designed to be
played as one-shot sound effects by a game's playroutine (see
`readme.txt §6.3` for the sfx data format). Load with `File → Load
Instrument…`. They demonstrate how compact a usable SID instrument can
be — under 100 bytes including all tables.

## Program files (`example1.prg`–`example4.prg`)

These are pre-packed C64-runnable demonstrations (load on a C64/VICE,
`SYS 4096` or similar). Not loadable in the editor — they exist to show
what a packed/relocated song looks like and to verify the export
pipeline.
