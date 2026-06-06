# In-app help specification — GoatTracker Qt frontend

Inventory of tooltips, popovers and contextual help strings the Qt frontend
should attach to its editable fields. Sourced from `readme.txt` sections
3.1–3.6 and the per-table legend in `qt/TablesView.cpp`.

Conventions:

* **Tooltip text** is the short hover string (max ~120 chars, two short
  lines OK). Implementation hint: `QWidget::setToolTip()`. Use plain ASCII
  hex prefixed with `$`, match the GoatTracker readme nomenclature.
* **Help link (F1)** is an anchor into `readme.txt` (or its rendered HTML
  equivalent) and/or `quickstart.md`. Implementation hint: a `whatsThis()`
  text plus an F1 keypress handler that opens the anchored doc.
* **Validation hint** is a soft warning to display in the status strip
  or as a transient tooltip when the user enters a known footgun value.
  Do not block the edit; just nudge.

---

## Song metadata view (`SongNameView`)

### Title (`title_`)
**Tooltip text:** Song title, up to 31 chars. Embedded in the .sng header and the packed .sid output. ISO-8859-1 only.
**Help link (F1):** readme.txt §6.1.1 Song header
**Validation hint:** Warn if length exceeds 31 chars (clipped on save).

### Author (`author_`)
**Tooltip text:** Composer credit. Stored in the .sid HVSC-style header. Use your handle/group, not a full name.
**Help link (F1):** readme.txt §6.1.1
**Validation hint:** None.

### Copyright (`copyright_`)
**Tooltip text:** Year and group/release, e.g. "2026 Covert Bitops". Stored in the .sid header.
**Help link (F1):** readme.txt §6.1.1
**Validation hint:** None.

---

## Instrument editor (`InstrumentView`)

### Instrument list (`list_`)
**Tooltip text:** Instruments $01–$3F. Instrument $00 is "no change" — selecting it in a pattern keeps the previous instrument running.
**Help link (F1):** readme.txt §3.3 Instrument data
**Validation hint:** Warn when editing instrument $00 — pattern cells cannot reference it.

### Name (`name_`)
**Tooltip text:** Instrument name, up to 15 chars. Shown in the instrument list and saved with the song.
**Validation hint:** None.

### Attack (`attack_`)
**Tooltip text:** SID envelope attack 0–F. 0 = instant rise (2 ms), F = slowest (8 s). Combined with Decay into ADSR byte $AD.
**Help link (F1):** readme.txt §3.3 ADSR diagram
**Validation hint:** If Attack=0 and the first wavetable step plays a real waveform, the first frame may be silent — suggest adding a delay step (`09 00` or `E9 00`) to the wavetable.

### Decay (`decay_`)
**Tooltip text:** SID envelope decay 0–F. 0 = instant drop to sustain, F = slowest (24 s). Low byte of the ADSR $AD register.
**Help link (F1):** readme.txt §3.3
**Validation hint:** None.

### Sustain (`sustain_`)
**Tooltip text:** Sustain level 0–F. 0 = silent (note fades to nothing during sustain), F = full volume.
**Help link (F1):** readme.txt §3.3
**Validation hint:** Warn if Sustain=0 and Release>0 — release won't be audible.

### Release (`release_`)
**Tooltip text:** Envelope release 0–F. 0 = instant cut on key-off, F = slowest fade.
**Help link (F1):** readme.txt §3.3
**Validation hint:** None.

### Wavetable Pos (`wave_`)
**Tooltip text:** Starting step in the Wave table for this instrument. $01–$FF. $00 stops wavetable execution (rarely useful).
**Help link (F1):** readme.txt §3.4.1 Wavetable
**Validation hint:** Warn if the target step's left byte is `00` (no-change) — first step must set a real waveform or notes may go missing.

### → table (Wavetable goto button)
**Tooltip text:** Jump to this step in the Wave table editor (F8).

### Pulsetable Pos (`pulse_`)
**Tooltip text:** Starting step in the Pulse table. $01–$FF. $00 leaves any currently-running pulse program untouched.
**Help link (F1):** readme.txt §3.4.2 Pulsetable
**Validation hint:** If you don't use pulse waveform, leave at $00 to save rastertime.

### → table (Pulsetable goto button)
**Tooltip text:** Jump to this step in the Pulse table editor.

### Filtertable Pos (`filter_`)
**Tooltip text:** Starting step in the Filter table. $01–$FF. $00 leaves any currently-running filter program untouched. Only one channel at a time should drive the filter.
**Help link (F1):** readme.txt §3.4.3 Filtertable
**Validation hint:** Warn if more than one instrument used simultaneously has a non-zero filter pointer in the current song — they will fight.

### → table (Filtertable goto button)
**Tooltip text:** Jump to this step in the Filter table editor.

### Vibrato Param (`vibParam_`)
**Tooltip text:** Index into the Speed table for instrument vibrato. Same semantics as pattern command 4XY. $00 disables instrument vibrato.
**Help link (F1):** readme.txt §3.4.4 Speedtable
**Validation hint:** Warn if pointer is non-zero but Vibrato Delay is $00 — instrument vibrato is off regardless.

### Vibrato Delay (`vibDelay_`)
**Tooltip text:** Ticks before instrument vibrato kicks in. $00 disables instrument vibrato entirely.
**Help link (F1):** readme.txt §3.3
**Validation hint:** None.

### HR/Gate Timer (`gateTimer_`)
**Tooltip text:** Hard-restart / gate-off timing, in ticks. Max value = tempo − 1. Bit $80 disables hard-restart; bit $40 disables gate-off (for legato).
**Help link (F1):** readme.txt §3.3
**Validation hint:** Warn if value (masked with $3F) is ≥ current song tempo — playback will halt.

### 1stFrame Wave (`firstWave_`)
**Tooltip text:** Waveform byte written on the very first frame of every new note. Usually $09 (gate+test). Special values: $00 = leave wave & gate alone, $FE = gate off, $FF = gate on.
**Help link (F1):** readme.txt §3.3 instrument legato
**Validation hint:** None.

### Play test note button
**Tooltip text:** Trigger the current instrument on the current octave. Uses Ch1 by default; mirror of pattern editor SHIFT+SPACE behaviour.

### Silence button
**Tooltip text:** Release-key the test note. Audible release phase will still play out.

---

## Tables editor (`TablesView`)

### Wave table — Left byte
**Tooltip text:** Wave step opcode. $00 hold, $01–$0F delay N frames, $10–$DF write to SID waveform reg, $E0–$EF inaudible wave, $F0–$FE run pattern cmd 0XY–EXY, $FF jump.
**Help link (F1):** readme.txt §3.4.1
**Validation hint:** Warn `$00` (no change) on step 1 of a referenced position — silent note.
**Validation hint:** Block `$08` not allowed first either (testbit-only with no waveform).
**Validation hint:** Warn if right neighbour points to an `$FF` jump cell — undefined behaviour.

### Wave table — Right byte (note)
**Tooltip text:** Note value. $00–$5F = positive relative semitones (added to played note). $60–$7F = negative relative (subtract 0–31). $80 = hold last freq. $81–$DF = absolute notes C#0–B7.
**Help link (F1):** readme.txt §3.4.1
**Validation hint:** Warn if relative offset takes a high played note past B7.

### Wave table — Right byte when left is `$FF`
**Tooltip text:** Jump target step (1-based). $00 = stop wavetable execution. Pointing at another $FF cell is undefined.
**Validation hint:** Highlight if target step's left byte is `$FF`.

### Wave table — Right byte when left is `$F0`–`$FE`
**Tooltip text:** Parameter for pattern command (L − $F0)XY. E.g. L=$F6 R=$5A runs cmd 6XY ($5A) → set sustain/release to $5A. Commands 0XY, 8XY and EXY are illegal here.
**Help link (F1):** readme.txt §3.4.1 example block

### Pulse table — Left byte
**Tooltip text:** $01–$7F modulate N ticks (right byte = signed speed). $80–$FE set pulse width high nybble (right byte = low 8 bits). $FF jump.
**Help link (F1):** readme.txt §3.4.2
**Validation hint:** None.

### Pulse table — Right byte when L is $01–$7F
**Tooltip text:** Signed 8-bit modulation speed added to pulse-width each tick. Positive = widen, negative = narrow.
**Validation hint:** None.

### Pulse table — Right byte when L is $80–$FE
**Tooltip text:** Low 8 bits of the 12-bit pulse-width value. Combined: PW = ((L & $0F) << 8) | R.
**Validation hint:** None.

### Pulse table — Right byte when L is $FF
**Tooltip text:** Jump target step (1-based). $00 = stop pulse execution.
**Validation hint:** Highlight if target is another $FF.

### Filter table — Left byte
**Tooltip text:** $00 set cutoff (right=cutoff). $01–$7F modulate cutoff (right=signed speed). $80–$F0 set passband+resonance+mask. $FF jump.
**Help link (F1):** readme.txt §3.4.3
**Validation hint:** None.

### Filter table — Right byte when L is $00
**Tooltip text:** Cutoff value $00–$FF written directly to SID register. 8-bit only; the lowest 3 bits are dropped on real hardware.
**Validation hint:** None.

### Filter table — Right byte when L is $80–$F0
**Tooltip text:** High nybble = resonance ($0–$F). Low nybble = channel filter-on mask (bit 0 ch1, bit 1 ch2, bit 2 ch3, bit 3 ext-in). Passband by L: $90 LP, $A0 BP, $B0 LP+BP, $C0 HP, $D0 LP+HP, $E0 HP+BP, $F0 all.
**Help link (F1):** readme.txt §3.4.3
**Validation hint:** None.

### Filter table — Right byte when L is $FF
**Tooltip text:** Jump target step (1-based). $00 = stop filter execution.
**Validation hint:** Highlight if target is another $FF.

### Speed table — Left byte
**Tooltip text:** Polymorphic. As vibrato: tick-count before direction flip. As portamento: high byte of 16-bit pitch delta. As funktempo: tempo for even rows. Bit $80 enables note-independent mode.
**Help link (F1):** readme.txt §3.4.4 Speedtable
**Validation hint:** None — semantics depend on what command references this row.

### Speed table — Right byte
**Tooltip text:** Vibrato: depth per tick. Portamento: low byte of 16-bit pitch delta. Funktempo: tempo for odd rows. Note-indep mode: right-shift divisor.
**Help link (F1):** readme.txt §3.4.4

### Speed table — Negate (SHIFT+N)
**Tooltip text:** Two's-complement the right byte. Use to flip vibrato direction or pulse/filter modulation speed.

### Speed table — Optimize (SHIFT+O)
**Tooltip text:** Merge duplicate entries and redirect references. Run before saving for smallest packed song.

### Speed table — Convert limit → time (SHIFT+L)
**Tooltip text:** Convert old-style limit-based pulse/filter modulation steps (where R was the limit) to new time-based steps. See readme §3.6.1.
**Help link (F1):** readme.txt §3.6.1

### Table lock (SHIFT+U)
**Tooltip text:** Toggle locked / unlocked table scrolling. When unlocked, each of the four tables remembers its own scroll position (status bar shows "U").

---

## Pattern editor (`PatternView`)

### Channel header — pattern number (P##)
**Tooltip text:** Pattern shown on this channel. Click to pick another pattern (00–CF). Same pattern can play on multiple channels.
**Help link (F1):** readme.txt §3.2 Pattern data
**Validation hint:** None.

### Channel header — pattern length (L##)
**Tooltip text:** Length in rows (decimal max 128 = $80). Set via SHIFT+L in pattern edit. Different channels can use different-length patterns.
**Validation hint:** Warn if length is 0.

### Channel header — mute toggle (M / ·)
**Tooltip text:** Mute this channel (does not stop SID register writes — only silences output). Click toggles; SHIFT+F4 mutes the focused channel.

### Pattern cell — Note column (`epcolumn = 0`)
**Tooltip text:** Note C-0…G#7. Special values: `...` rest (no event), `===` key off (release), `+++` key on (re-gate without re-fetching pitch). ProTracker note-entry layout (ZSXDCV…).
**Help link (F1):** readme.txt §3.2
**Validation hint:** Warn if a note above G#7 is encoded (use orderlist transpose instead).

### Pattern cell — Instrument column (`epcolumn = 1,2`)
**Tooltip text:** Two-digit hex $01–$3F. $00 = "no change" (keep last instrument running). Cleared with `.`.
**Validation hint:** Warn if a note is placed without ever having set an instrument earlier in the channel.

### Pattern cell — Command digit (`epcolumn = 3`)
**Tooltip text:** 0=nop  1=porta up  2=porta down  3=tone porta  4=vibrato  5=set AD  6=set SR  7=set wave  8=wavetable ptr  9=pulsetable ptr  A=filtertable ptr  B=filter ctl  C=cutoff  D=mastervol/timing  E=funktempo  F=tempo.
**Help link (F1):** readme.txt §3.2 pattern command reference
**Validation hint:** None.

### Pattern cell — Command parameter (`epcolumn = 4,5`)
**Tooltip text:** Two-digit hex argument. For 1XY–4XY this is a speed-table index, not the slide rate itself. Press SHIFT+RETURN here to convert an old-style raw value into a fresh speedtable entry.
**Help link (F1):** readme.txt §3.4.4
**Validation hint:** For cmd 1/2/3/4/E, warn if the index points to an empty speed-table row.
**Validation hint:** For cmd F (tempo), warn if value < $03 (will be reinterpreted as funktempo recall).

### Pattern cell — cmd `1XY` (portamento up)
**Tooltip text:** Slide pitch up. XY is a Speed-table index, NOT a slide rate. The table cell's 16-bit value is added to pitch each tick.

### Pattern cell — cmd `2XY` (portamento down)
**Tooltip text:** Slide pitch down. XY is a Speed-table index. Subtracts the 16-bit value each tick.

### Pattern cell — cmd `3XY` (tone portamento)
**Tooltip text:** Slide toward the most recent target note. XY indexes the Speed table. XY=$00 = tie-note (instant pitch warp, no slide).

### Pattern cell — cmd `4XY` (vibrato)
**Tooltip text:** Vibrato. XY indexes the Speed table; that row's L=speed, R=depth. Overrides instrument vibrato while active.

### Pattern cell — cmd `5XY` (set AD)
**Tooltip text:** Write XY directly to the SID Attack/Decay register. X=attack, Y=decay. One-shot; envelope keeps using new value until reset.

### Pattern cell — cmd `6XY` (set SR)
**Tooltip text:** Write XY directly to the SID Sustain/Release register. X=sustain level, Y=release.

### Pattern cell — cmd `7XY` (set waveform)
**Tooltip text:** Direct write to the SID waveform/control register. Ineffective if the wavetable is also changing waveform on the same tick.

### Pattern cell — cmd `8XY` (wavetable pointer)
**Tooltip text:** Jump the wavetable to step XY. $00 stops wavetable execution. Combine with legato (HR/Gate $40 + 1stFrame $00) for per-note waves.

### Pattern cell — cmd `9XY` (pulsetable pointer)
**Tooltip text:** Jump the pulsetable to step XY. $00 stops pulse execution — handy for non-pulse instruments.

### Pattern cell — cmd `AXY` (filtertable pointer)
**Tooltip text:** Jump the filtertable to step XY. $00 stops filter execution.

### Pattern cell — cmd `BXY` (filter control)
**Tooltip text:** Set SID filter control register. X=resonance ($0–$F), Y=channel filter-on bitmask. $00 turns filter off entirely.
**Help link (F1):** readme.txt §3.2 command BXY

### Pattern cell — cmd `CXY` (filter cutoff)
**Tooltip text:** Set filter cutoff to XY. Ineffective if filtertable is also writing cutoff on the same tick.

### Pattern cell — cmd `DXY` (master volume / timing)
**Tooltip text:** If X=0: set master volume to Y. If X≠0: write XY to player address $3F (used for sync/timing marks in HVSC tools).

### Pattern cell — cmd `EXY` (funktempo)
**Tooltip text:** Activate funktempo using Speed-table row XY. L=tempo for even rows, R=tempo for odd. Sets on all channels; override per-channel with FXY ≥ $83.
**Help link (F1):** readme.txt §3.4.4 funktempo

### Pattern cell — cmd `FXY` (tempo)
**Tooltip text:** Set tempo. $03–$7F = all channels. $83–$FF = current channel only (sub $80 for actual value). $00–$01 = recall funktempo.
**Validation hint:** Block $02 — fastest legal tempo is $03 (or $02 via funktempo with caveats; see readme §3.6).

### Follow-play toggle (Ctrl+F)
**Tooltip text:** When ON, the edit cursor tracks the playhead. Useful for debugging; turn OFF to keep editing while music plays.

### Record toggle (space)
**Tooltip text:** Toggle pattern record. When ON, keypresses write notes; when OFF, the keyboard just auditions.

### Speed multiplier (SHIFT+F5 / SHIFT+F6)
**Tooltip text:** Player calls per frame. 1x = standard 50/60 Hz. 2x–8x trade rastertime for finer effect resolution.

### SID model toggle (SHIFT+F8)
**Tooltip text:** Switch reSID emulation between 6581 (warmer, filter drift, classic Hubbard/Galway sound) and 8580 (cleaner, accurate filter, modern norm). Only affects emulation, not the song data.

---

## Order list editor (`OrderView`)

### Order cell — pattern number ($00–$CF)
**Tooltip text:** Pattern to play at this song position. Editable with hex digits; +/- increments.
**Help link (F1):** readme.txt §3.1 Orderlist data
**Validation hint:** None.

### Order cell — TRANSPOSE (+X or -X)
**Tooltip text:** Transpose subsequent patterns. +0–+E semitones up, -1–-F down. Stays active until another TRANSPOSE or song restart. Must appear BEFORE any REPEAT.
**Help link (F1):** readme.txt §3.1
**Validation hint:** Warn if a TRANSPOSE follows a REPEAT in the same chain — engine will halt playback.
**Validation hint:** Reminder: transpose is NOT reset on RST loop. Reset manually at the top of the loop if needed.

### Order cell — REPEAT (RX)
**Tooltip text:** Repeat the next pattern X+1 times. R0 = 16 times. Must come AFTER any TRANSPOSE for the same pattern.
**Help link (F1):** readme.txt §3.1
**Validation hint:** Warn if a REPEAT is immediately before RST (endmark must be a pattern number, not a command).

### Order cell — RST endmark
**Tooltip text:** Restart marker. The byte to its right is the restart position (0-based step in this orderlist). Loop point of the subtune.
**Help link (F1):** readme.txt §3.1
**Validation hint:** Warn if restart position points past the end of this channel's orderlist.

### Subtune selector (`< >` keys)
**Tooltip text:** Switch between subtunes 1–32. Each subtune has its own 3-channel orderlist but shares patterns, instruments and tables.
**Help link (F1):** readme.txt §3.1

### Channel swap (SHIFT+1/2/3)
**Tooltip text:** Swap the active channel's orderlist with channel 1/2/3 in this subtune. Patterns themselves do not move.

---

## Status strip / globals

### Speed multiplier indicator (`MUL`)
**Tooltip text:** Frame-rate multiplier. 1x = NTSC/PAL frame rate. Higher = more ticks per row → finer modulation resolution but more CPU on real C64.

### SID model indicator (`SID`)
**Tooltip text:** Active reSID model: 6581 (R3, MOS warm) or 8580 (CSG cool). Toggle with SHIFT+F8.

### PO (pulse optimization) toggle
**Tooltip text:** When ON, pulse-table execution is skipped on certain ticks to save rastertime. Turn OFF (`/O0`) for tempo-3 tunes that need every pulse tick. Same as readme §3.5.

### RO (realtime command optimization) toggle
**Tooltip text:** When ON, realtime cmds 1XY-4XY skip on tick 0. Turn OFF (`/R0`) for jitter-free portamento/vibrato at the cost of rastertime.

### Hardrestart / multispeed default selectors
**Tooltip text:** Default value used by F11/F12 file save for the packed/relocated output. Has no effect on .sng playback in the editor.

---

## Instrument quick-list dock (`InstrumentQuickList`)

### List entry
**Tooltip text:** Click to select; double-click to jump to instrument editor with this entry loaded. Right-click for context: copy / paste / clear / save as .ins / load .ins.

---

## Order minimap dock (`OrderMiniMap`)

### Minimap area
**Tooltip text:** Tile-by-tile view of the active subtune's orderlist. Color = pattern hash. Click to scroll the order editor to that position.

---

## File menu

### Open .sng…
**Tooltip text:** Open a GoatTracker v2 song file. Only .sng can be loaded back into the editor; .prg/.bin/.sid are export-only.
**Help link (F1):** readme.txt §6.1 file format

### Save / Save As
**Tooltip text:** Save as .sng (editor format). Use the dedicated relocator/packer for runtime exports.
**Validation hint:** Warn the user once per session if they try to save a file with no extension or an extension other than .sng.

### Load / Save Instrument (.ins)
**Tooltip text:** Single-instrument file: includes ADSR, table pointers, AND all four tables (wave/pulse/filter/speed) needed by that instrument. Loading appends the instrument's table data to the song's tables.
**Help link (F1):** readme.txt §6.2 .INS format
**Validation hint:** Warn if loading would overflow the song's free table rows.

---

## Notes for implementation

* Most spinboxes can carry a static tooltip via `setToolTip`. Polymorphic
  fields (the Wave-table left/right pair, the pattern command parameter)
  need *dynamic* tooltips computed from current row state — wire them
  through `event(QEvent::ToolTip)` and inspect `ltable[t][etpos]` /
  `pattern[][]` to decide the message.
* The Validation hints map naturally onto a single status-strip message
  channel: a soft yellow line on edit that auto-clears after 4 s. Avoid
  modal dialogs for any of these.
* Help link (F1) anchors require turning `readme.txt` into rendered HTML
  or markdown with explicit `<a name>` anchors. As an interim measure, a
  `QDesktopServices::openUrl` to a covertbitops.c64.org or local file URL
  is fine.
* The Qt build already has a per-table legend and a live cell decoder in
  the tables view (`decodeCell()` in `qt/TablesView.cpp`). These are
  effectively in-line tooltips already — extend the same pattern to the
  pattern editor (live decode of `cmd param` in the status strip).
