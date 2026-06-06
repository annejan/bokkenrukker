# Stereo / 6-Channel Port Plan

This document plans the work needed to make our Qt frontend (which
currently drives the single-SID GoatTracker 2 core) support optional
dual-SID, 6-channel playback as in GoatTracker 2 Stereo Edition. It is
a survey + plan, not an implementation.

Reference source tree:
`/home/paul/work/c64/goattracker2-code/gt2stereo/trunk/src/` (v2.77 Stereo).

## 1. Source-tree diff summary

Identical or trivially different (whitespace / formatting only):

- `bme/`, `asm/`, `palette.bin`, `chargen.bin`, `cursor.bin`, `cwsid.h`,
  `gfile.c/h`, `gtable.c/h`, `ginstr.c`, `gsong.h`.

One-line, mechanical changes:

- `gcommon.h`: `MAX_CHN 3` -> `MAX_CHN 6`. That is the **only** diff in
  that header. All array shapes (`pattern[]`, `songorder[]`, `chn[]`,
  `songlen[]`, `epnum[]`, `espos[]`) are already `[MAX_CHN]`-sized in
  the headers, so the data model widens by recompile.

Substantively forked:

- `gsid.cpp/h` - two reSID instances, two register banks, stereo
  buffer fill API.
- `gsound.c` - allocates `lbuffer`/`rbuffer`, opens SDL with
  `SIXTEENBIT|STEREO`, interleaves L/R, dual HardSID routing.
- `gplay.c` - second filter state (`filter2ctrl/type/cutoff/time/ptr`)
  and a per-row `if (c < 3) sidreg[] else sidreg2[]` split on every
  register write. Chn-3..5 tempo and ADSR handling extended.
- `gsong.c` - the .sng format stays "GTS5" but the loader now sniffs
  the orderlist endmark to detect mono vs stereo and pads channels
  4..6 with an empty pattern when loading a mono file. Adds
  `determinechannels()` and `mergesong()` helpers.
- `greloc.c` - splits `sidaddress` into low word (SID1, $D400) and
  high word (SID2, $D500). `insertdefine("SIDBASE", ...)` and
  `insertdefine("SID2BASE", ...)`. PSID version bumped to 0x0003 and
  the SID-model byte sets bits for both chips. The 3-channel
  optimization paths (`if (!chnused[2]) channels = 2;`) are removed -
  stereo always emits 6 channels.
- `player.s` / `altplayer.s` - second filter state (`mt_filt*_sid2`,
  `mt_tick0_a/b/c_sid2`) and a parallel `SID2BASE` register-write
  block. Same playroutine, forked rather than parameterized.
- `goattrk2.h`, `gt2stereo.c` - add `monomode` (runtime sum-to-mono
  toggle for SDL output), `sidaddress` widened to 32 bits.

New only in stereo: `gt2stereo.c/.rc/.ico/.seq`, `ss2stereo.c`
(pattern splitter), `mod2sng2.c`, `bcursor.bin/hex` (block cursor).

New only in our mono tree: `goattrk2.c` (replaced by Qt main),
`sngspli2.c`, `betaconv.c`, `mod2sng.c`, `gsid.cpp.old` (pre-libresidfp
backup), our `qt/` directory.

## 2. Song-format compatibility

The stereo build uses the **same** "GTS5" magic as mono. It does not
introduce "GTSS" or "GTS6". The file format extension is:

- After the three name strings, the loader reads the songtable. For
  each song it tries to read MAX_CHN (=6) orderlists. A mono file
  only has 3 orderlists per song, so the loader peeks ahead via
  `determinechannels()` (fseek save/restore, parse orderlist sizes,
  check end-mark byte 0xFF and the trailing transpose byte for
  plausibility) and decides 3 vs 6 at runtime.
- If a mono song is detected, channels 4..6 are filled with an empty
  pattern selected from `pattused[]`.
- The rest of the file (instruments, patterns, wavetable, etc.) is
  layout-compatible byte-for-byte.

Implications for our Qt build:

- A **mono .sng loaded by a stereo build** works (it pads).
- A **stereo .sng loaded by a mono build** does **not** work - the
  loader will misread the songtable after the first 3 orderlists. We
  inherit the upstream "no magic version bump" decision; the only way
  to refuse stereo songs cleanly is to do the same sniff and bail.
- No header constants need to change. If we want to add a friendlier
  fast-path we could optionally introduce a "GTSS" or "GTS6" magic at
  save-time (rejected upstream so we should not diverge here).

## 3. Engine changes

- Two `reSIDfp::SID` instances, owned by `gsid.cpp`. Our current
  single-instance pointer becomes a pair `sid` / `sid2`. Two register
  shadow arrays (`sidreg[]`, `sidreg2[]`).
- Stereo SID address is configurable via `sidaddress`, encoded as
  `(SID2_LO << 16) | SID1_LO` -> default `0xD500D400`. Common other
  layouts the user might want: `0xDE00D400` (REU/cartridge area),
  `0xD500D400` (8580+stereo expander), `0xD420D400` (rare hardware).
  At ASM relocation time, `greloc.c` inserts both `SIDBASE` and
  `SID2BASE` defines.
- Playroutine is **forked** not parameterized: `player.s` /
  `altplayer.s` contain literal `SID2BASE` writes alongside the
  existing `SIDBASE` writes; second filter state lives in
  `mt_filt*_sid2` labels. We bring those forked .s files in as part
  of the data file we link via `bme/datafile/`.
- Audio mixing: `sid_fillbuffer(short *lptr, short *rptr, int samples)`
  produces two separate streams; `gsound.c` interleaves L/R for SDL
  and routes to two HardSID device IDs when applicable. We render to
  a stereo SDL audio device (`SIXTEENBIT|STEREO`) or sum to mono at
  the last step when `monomode != 0`.

## 4. Concrete patch plan

In dependency order. Sizes: **S** ~1h, **M** half day, **L** day+.

1. **S** `src/gcommon.h`: bump `MAX_CHN` to 6 (compile-time switch -
   see section 6). Verify the `[MAX_CHN]` array declarations in
   `gsong.h`, `gplay.h` widen as expected.
2. **M** Replace `src/gsid.cpp` with a libresidfp-backed two-instance
   version. Add `sidreg2[]`, `sid2`, dual `envelopeLevel()` reads,
   change `sid_fillbuffer` signature to (lptr, rptr, samples). Add a
   `sid2_setbase(uint16)` if we want runtime address selection.
3. **M** `src/gsound.c`: allocate `lbuffer/rbuffer`, request a stereo
   SDL device, interleave, honour `monomode`. Drop the
   `sid_getlevels` 3-element assumption (return 6).
4. **L** `src/gplay.c`: import the `(c < 3) ? sidreg[...] : sidreg2[...]`
   split on every register write, the `filter2*` state machine, the
   per-MAX_CHN tempo / songinit / espos loops. This is the bulk of
   the C-side delta.
5. **S** `src/gsong.c`: import `determinechannels()` + the mono->stereo
   padding path so we transparently load both. Keep "GTS5" magic.
6. **M** `src/greloc.c`: split `sidaddress` into two halves, emit
   `SID2BASE`, drop the 3-channel optimizer paths, bump PSID v3 + dual
   SID-model bits when exporting .sid.
7. **M** Drop in stereo `src/player.s` / `src/altplayer.s` (and the
   matching `bme/datafile/`). Regenerate the linked data blob.
8. **S** `qt/qt_globals.c`: change `sidaddress` default to
   `0xd500d400u`, add `monomode` global.
9. **M** `qt/PatternView.cpp`: widen `unsigned char levels[3]` -> `[6]`,
   widen `scope_[3]` -> `[6]`, and convert the two `for (c=0; c<3)`
   loops to `MAX_CHN`. Reduce per-channel slot width or enable
   horizontal scroll. Channel header strip mute hit-tests already
   iterate `MAX_CHN`, so they recompute. Re-check `chnW_` math.
10. **S** `qt/OrderView.cpp` & `qt/OrderMiniMap.cpp`: already
    MAX_CHN-driven. Re-check column widths so 6 columns still fit
    the order dock.
11. **S** `qt/StatusStrip.cpp`: already MAX_CHN-driven (it indexes
    by `epchn`). No structural change; widen channel selector
    if it has fixed 3-way switch.
12. **S** `qt/UndoStack.cpp`: snapshot struct already shaped by
    `[MAX_CHN]`. Just verify byte-size growth on undo entries is
    acceptable (~2x for orderlists).
13. **S** `qt/MainWindow.cpp`: surface the new "SID2 base address" and
    "Mono output" toggles in the audio settings menu.

## 5. UX implications

- **Pattern view**: at 6 channels and current `chnW_` the view will
  exceed any reasonable window width. Two viable options: (a) shrink
  `chnW_` ~30% (drop the gap, narrow effect column) and keep all 6
  on-screen, or (b) keep 3 visible at a time and add a Page-Right
  shortcut + horizontal scroll. Option (a) matches upstream
  `gpattern.c` behavior; option (b) is friendlier on small monitors.
  Recommend (a) by default with `bigwindow`-style auto-(b) for narrow
  windows.
- **Order dock**: 6 narrow columns instead of 3 - still fits the
  dock, but `OrderMiniMap` colW math will halve.
- **Mute hit-tests**: already loop `MAX_CHN` in `PatternView.cpp`
  header strip; only the `levels[3]` array and the `for (c=0; c<3)`
  scope loop need to widen.
- **Status strip "Channel N"**: it indexes `epchn` so the label just
  reads "Channel 5", but the prev/next chord shortcuts need to
  understand 6 instead of 3.
- **InstrumentView / TablesView / SongNameView**: not channel-indexed,
  no change.

## 6. Compile-time vs runtime mode

Recommend **compile-time**, separate target. Rationale:

- Player ASM is forked, not parameterized; switching at runtime would
  require shipping both player blobs and a relocator that picks one.
- The PSID export format, file-load sniffer, and SID2 address handling
  all assume one channel count for the whole session.
- Memory and UI tuning (`chnW_`, scope buffer width, undo entry size)
  benefit from a known constant.
- Songs themselves already self-describe via the orderlist sniff -
  the *binary* will refuse a stereo song when built mono, which is
  the safe default. A user wanting stereo runs the stereo build.

Concretely: CMake option `GOATTRK2_STEREO` (default OFF). When ON,
defines `MAX_CHN=6`, swaps `gsid.cpp/gplay.c/gsong.c/greloc.c/gsound.c`
for the stereo variants, links the stereo player blob, and renames the
target to `goattrk2-qt-stereo`. The Qt source files use `MAX_CHN`
everywhere already, so they compile both ways from one tree.

## 7. Biggest risk

The **playroutine fork**. `player.s` and `altplayer.s` in the stereo
build are not parameterized - they contain literal `SID2BASE+$xx`
writes and a duplicated filter state machine. They are also the
runtime that the exported .sid/.prg uses to drive real hardware, so
any deviation from upstream produces silent-but-wrong songs.

Mitigation:

- Vendor the upstream stereo `.s` files verbatim into `src/`.
- Reuse upstream's `bme/datafile/` build pipeline rather than hand-
  editing the linked data blob.
- Smoke-test by exporting a known stereo example song to .prg and
  diffing the binary against a fresh upstream `gt2stereo` export
  byte-for-byte (the relocator is deterministic).
- Add a CI check: load each `examples/*.sng`, export, hash, fail on
  drift.

## 8. Sources read

Files in `gt2stereo/trunk/src/` actually opened (or diffed against
our mono tree) for this plan:

- `gcommon.h`
- `gsong.c`, `gsong.h`
- `gsid.cpp`, `gsid.h`
- `gsound.c`, `gsound.h`
- `gplay.c`, `gplay.h`
- `gpattern.c`
- `gorder.c`
- `greloc.c`, `greloc.h`
- `player.s`, `altplayer.s`
- `goattrk2.h`, `ginstr.h`
- `gt2stereo.c`
- `ss2stereo.c` (header only)

And on the Qt side:
`qt/PatternView.cpp`, `qt/OrderView.cpp`, `qt/OrderMiniMap.cpp`,
`qt/StatusStrip.cpp`, `qt/UndoStack.cpp`, `qt/CMakeLists.txt`,
`qt/qt_globals.c`.
