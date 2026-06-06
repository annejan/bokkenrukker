# Modern Tracker UX Patterns for the goattracker Qt Frontend

Research notes and a prioritised backlog for the Qt6 frontend under `qt/`.
Focus: workflow features that demonstrably save composer time or prevent
mistakes; visual polish is mentioned only where it materially changes use.

The frontend (June 2026) ships `PatternView`, `OrderView`, `OrderMiniMap`,
`InstrumentView` (ADSR + wavetable preview), `InstrumentQuickList`,
`TablesView` (wave/pulse/filter/speed), `StatusStrip`, `SongNameView`, and a
recent MIDI jam mode. There is no undo, no piano roll, no command palette, no
cross-song instrument library, no automation lane.

---

## 1. Executive summary

Five features that would have the largest single-user-session impact, ranked:

1. **Undo / redo with a memento ring buffer.** Every other modern tracker has
   it; goattracker's lack of it is its single biggest "scary to edit live"
   problem. Cheap to implement on top of the existing flat song structures.
2. **Multi-row, multi-channel marquee selection with mix-paste / interpolate /
   transpose.** Renoise- and OpenMPT-style block ops are the difference between
   "rewrite this 32-row section" being a minute vs. a half-hour.
3. **Cross-song instrument library panel.** A dockable browser over a user
   directory of `.ins`/`.gti` files with drag-to-slot. Composers reuse SID
   leads, basses, and drum kits across tracks; today every reuse requires a
   File-Open round trip.
4. **Searchable command palette (Ctrl+P).** One Qt widget, replaces 40% of the
   menu bar in practice, makes every shortcut discoverable. Renoise's most
   praised power-user feature.
5. **Per-channel oscilloscope strip plus a "what's playing" hover overlay.**
   The existing scope ring buffer is the foundation; turning it into a
   first-class playback feedback widget is mostly UI work.

These five items are the "do these first" backlog. Everything in section 5
extends them.

---

## 2. Per-tracker findings

### Renoise (commercial, modern)

The reference for "tracker that became a hybrid DAW". Workflow grew
incrementally rather than by rewriting the tracker model.

- **Pattern Matrix.** Bird's-eye block grid; each cell is "this pattern on
  this channel". Drag to select, Ctrl+drag to copy, Ctrl+Shift to insert into
  new patterns. Solves the "swap pattern 03/04 on channel 2 only" case.
- **Mix-paste.** Paste only fills empty cells. Essential for non-destructive
  layering.
- **Graphical automation lanes.** Right-click-drag a parameter while playing,
  down to 1/256 row resolution. SID has no VST analogue, but the same widget
  could draw filter cutoff / pulse width over rows.
- **Edit step quick-adjust.** Ctrl+1..0 sets row advance; Ctrl+plus/minus
  nudges. Sounds trivial; in practice the single most-used shortcut.
- **Command palette** (third-party but officially praised). Ctrl+P fuzzy
  search across "go to line", "set BPM", "select instrument".

### OpenMPT (open source, ProTracker descendant)

Benchmark for "old tracker done right". Conservative defaults, no piano
roll, but unmatched pattern editing operations.

- **Mix Paste / Push-Forward Paste / Paste Flood.** Three named paste modes
  covering merge, insert, and repeat-to-end-of-pattern, each with a shortcut.
- **Selection transforms: Interpolate, Transpose, Amplify, Grow/Shrink.**
  Interpolation between two volume/effect values is the killer feature for
  SID filter sweeps.
- **100,000-step undo per pattern and per sample.** Memento depth is not the
  bottleneck; correctness is.
- **Fully rebindable keyboard with FT2 / IT / Dvorak presets** and a
  searchable key-map table with conflict detection.
- **Quick clipboard.** A second one-event clipboard for stamping a copied
  effect cell without losing the main selection.

### MilkyTracker (FastTracker II clone)

Smallest UX surface of the modern lot, but two interesting affordances.

- **Two edit modes ("MilkyTracker" and "FT2").** Newcomers get Ctrl-C/X/V
  while legacy users keep muscle memory. Useful posture for goattracker,
  whose default key map is famously idiosyncratic.
- **Multi-tab modules.** Up to 32 songs open at once with paste between them.
- **Laptop-friendly nav (Ctrl+Up/Down for instrument switch).** Important on
  small screens.

### FamiStudio (modern NES tracker)

The only mainstream tracker shipping both a tracker grid **and** a piano
roll over the same data.

- **Piano roll alongside the tracker view.** Clicking a pattern in the
  sequencer scrolls the piano roll to that location.
- **Drag-to-set note duration.** Visual note bars; resize by dragging the
  right edge. Maps poorly to SID (gate-on/gate-off, not duration) but the
  visual length cue helps reading.
- **Envelopes drawn graphically.** Volume, arp, pitch as pen-tool curves;
  beats numeric SID table entry for users who think in shapes.
- **Quick Access Bar (mobile influence).** Docked column with frequent
  actions; goattracker Qt could use a similar always-visible sidebar.
- **MIDI / QWERTY recording with input quantization.**

### Furnace (recent multi-chip tracker, SID support)

The most actively developed alternative to goattracker for SID. The project
has explicitly *rejected* a piano roll as incompatible with the tracker
idiom; worth weighing seriously.

- **Per-channel + global oscilloscope** with waveform centering. Partial fit
  for goattracker's existing scope buffer.
- **Customisable dockable layout.** Every panel is a draggable tab. Qt's
  `QDockWidget` already supports this — the work is saving/restoring layouts.
- **Macros / per-tick parameter automation inside instruments.** Far more
  powerful than goattracker's wave/pulse/filter tables.
- **Channel oscilloscope click-to-mute / solo** for one-click live mix.
- **Note input options including chord input and split keyboards.**

### DefleMask (multi-chip, SID-capable)

Closed-source but widely studied; SID quirks handled particularly well.

- **Live edit during playback** — every edit reflected on the next tick.
- **Floating, draggable instrument editor** that stays visible across panes.
- **ADSR Hard Reset Time as a first-class parameter.** Names the workaround
  instead of hiding it in a table.
- **Real-time MIDI input with chord-input mode.**

### BambooTracker (YM2608)

Most relevant feature is its envelope editor.

- **Mouse-driven envelope shaping** — click-to-add, drag-to-move, right-
  click-to-delete. Direct upgrade path for goattracker's numeric tables.
- **Paste MML / text envelope formats** (8 pre-defined). goattracker
  equivalent: paste a hex blob into the wavetable.
- **Performance sequences with loop/release points** editable as graphical
  handles.

### Niche / honourable mentions

- **Polyend Tracker.** Per-step quantize independent of song quantize;
  chord-mode where one MIDI chord becomes one step. Suggests a "MIDI chord
  captures into arpeggio table" workflow for SID.
- **Reaper-style step input.** Notes insert without playhead advancing until
  shift releases — handy for SID arpeggio fragments.

---

## 3. Feature-by-feature deep dive

### 3.1 Piano roll vs tracker grid

FamiStudio shows both representations can coexist on one data model;
Furnace's maintainers reject it as incompatible with the tracker idiom.
SID-specific reality: notes have no native duration (gate-on/gate-off + table
tick), so a piano roll mostly visualises *when* notes start and how long the
gate holds. Helpful for reading; marginal for entering.

**Recommendation: Optional toggle, read-only first.** A `PianoRollView` next
to `PatternView` (`QStackedWidget` page or second dock) renders pattern rows
as horizontal note bars derived from existing song data. Read-only in v1:
clicking a bar scrolls the underlying pattern row. Widget: `QGraphicsView`
with `QGraphicsRectItem` per note. Data: existing `pattern[][]`,
post-processed into note-on..note-off runs.

### 3.2 Instrument browsing / library

No mainstream tracker has a great cross-song library. Renoise and DefleMask
rely on drag-from-disk; OpenMPT keeps an Instrument Library tree.
goattracker's `.ins`/`.gti` files already exist on disk and are tiny.

**Recommendation: Yes.** Add a dockable `InstrumentLibraryDock` watching
user-configured directories (`QFileSystemWatcher`), showing them as a flat
list with ADSR thumbnails via the existing `AdsrPreview` widget, supporting
drag-drop onto `InstrumentQuickList`. Selecting previews on the focused
channel (reuse the test-note path). Persist favourites in `QSettings`.

### 3.3 Pattern selection / clipboard

OpenMPT is the gold standard for pattern ops; Renoise's Pattern Matrix is the
standard for order-list block edits. Mix-paste, push-forward paste, and
interpolate-on-selection are universally praised. Multi-channel rectangular
marquee is table stakes.

**Recommendation: Yes, high priority.** Extend `PatternView` with rectangular
selection (Shift+arrows or mouse drag), cut/copy/paste/mix-paste/push-forward-
paste via internal `QMimeData`, plus three transforms: Interpolate (linear
between first/last non-empty cell), Transpose (semitone), Amplify (volume
scale). Add a Quick Clipboard `Ctrl+Shift+C/V` second register.

### 3.4 Undo / redo

Memento pattern with a bounded deque is standard. OpenMPT keeps 100,000
entries per pattern. With SID songs running ~64KB, even 1,000 snapshots is
cheap. Edit grouping is the subtle but vital part — without it, Ctrl+Z
removes only one character.

**Recommendation: Yes, top priority.** Use Qt's `QUndoStack` / `QUndoCommand`
to get redo and merging for free. Snapshot the affected region only
(pattern bytes / instrument struct / table row), never the full song. Each
command's `id()` identifies "same edit at same cell" so consecutive single-
key edits coalesce. Bind to `Ctrl+Z` / `Ctrl+Y`.

### 3.5 Real-time playback feedback

Furnace's per-channel and global oscilloscopes are universally loved.
Renoise highlights the playing row with a scrolling colour bar. FamiStudio
shows a piano-roll playhead at video frame rate.

**Recommendation: Yes, building on what exists.**
`PatternView::tickScope()` and the `scope_` ring buffer already exist.
Surface them: (a) larger per-channel scope strip, (b) click-to-mute on the
scope header, (c) global oscilloscope in `StatusStrip`, (d) playhead bar
animated via `QPropertyAnimation` instead of snapping per tick.

### 3.6 MIDI input mapping

DefleMask, Renoise and Polyend all support chord-input mode, velocity-to-
volume mapping, and CC→effect mapping. goattracker Qt has basic MIDI jam;
the natural extension is recording.

**Recommendation: Yes.** Build on the existing jam path: a chord-input
toggle (keys accumulate into the current row across channels until all
release, then advance), optional velocity→instrument-volume (off by default;
many SID composers want fixed dynamics), and a MIDI Input dialog listing
detected inputs with a test pad and Save/Load preset (`QSettings`). CC→table
mapping is v2.

### 3.7 Sample / waveform import

N/A literally (SID has no sample channel beyond 4-bit `$1F` tricks). The
wavetable program *is* the SID analogue. BambooTracker's MML paste is the
model.

**Recommendation: Optional.** A "Wave table presets" `QMenu` in `TablesView`
with built-in shapes (saw, square, ringmod sweep, hard-restart, classic
Hubbard pulse) plus "Paste hex" / "Copy hex" on the wavetable column.
Starter presets ship in `examples/wavetables/` as small `.gti` fragments.

### 3.8 Templates and starter songs

No tracker does this brilliantly. Renoise has a user-set default song.
Composers benefit most from a handful of song templates (PAL/NTSC,
single/multi-speed, stereo) and an instrument preset pack.

**Recommendation: Yes, small effort.** `File → New From Template…` backed by
`examples/templates/*.sng`. Ship 4 templates. Persist user templates in
`$XDG_CONFIG_HOME/goattracker/templates/`.

### 3.9 Keyboard shortcuts cheat sheet

Renoise's command palette is its single most re-shared workflow tip. OpenMPT
exposes bindings as a searchable Preferences table. MilkyTracker has a
printable PDF.

**Recommendation: Yes, both.** `Ctrl+P` opens a `CommandPalette` (`QDialog`
with `QLineEdit` + `QListView` over a `QSortFilterProxyModel`). Each command
has a title, optional shortcut, callback, and optional numeric argument
capture ("Go to row 32" parses the digit suffix). Add a
`Help → Shortcut Reference` window rendering the canonical key map; both
share the same central command table so they cannot drift.

### 3.10 Performance mode / live tweaking

Furnace lets you click an oscilloscope to mute. Renoise has a Live tab.
OpenMPT arms channels for live mute. Tempo nudge during playback is
universal.

**Recommendation: Yes.** A `PerformanceBar` (compact `QToolBar`, toggleable
from View) with mute/solo per channel, a live tempo spinbox bound to the
play-routine tempo, master volume, and an "edit lock" toggle that disables
accidental keypress editing during a show.

---

## 4. Anti-patterns to avoid

Modern trackers occasionally make choices that look slick in demos but cost
composers time. goattracker Qt should not copy these:

- **Mouse-only edits.** Drawing envelopes is fine as an option; if it becomes
  the only way to edit a table, keyboard users (the majority) suffer. Always
  keep numeric entry.
- **DAW-style infinite tracks.** SID is 3 channels (6 stereo). A reorderable
  track list is wasted complexity.
- **Floating, modal, undockable instrument editors.** Always-dockable, always-
  keyboard-reachable wins.
- **Tab-vomit.** Furnace's ImGui freedom lets users hide panels they need.
  Ship a *named default layout* and a one-click "Reset layout".
- **Undiscoverable shortcuts** (the SDL UI's F12-help-if-you-know-F12 trap).
  The command palette + searchable shortcut table fixes this.
- **"Smart" auto-quantize on keyboard typing.** Quantize is fine on MIDI
  record; never on step entry.
- **Hidden state in modal dialogs.** Settings should live in dockable panels
  or persistent menus, not `QDialog`s you cannot keep open while editing.
- **Breaking the song file format.** Every Qt feature must round-trip with
  the SDL frontend; no proprietary metadata in `.sng`. Store UI state in
  `QSettings`.

---

## 5. Prioritised backlog

Each item: title, 2-sentence description, complexity (S/M/L), expected user
time saved per session.

1. **Undo / redo via `QUndoStack`** — *M, saves 5–10 min/session.* Wrap
   every pattern, instrument, and table mutation in a `QUndoCommand` with
   merge IDs so consecutive same-cell edits coalesce. Biggest confidence
   booster for live editing.
2. **Marquee selection + mix-paste + push-forward-paste in `PatternView`**
   — *M, saves 3–8 min.* Mouse-drag and Shift+arrow rectangle selection
   across channels; cut/copy/paste plus two extra paste modes via internal
   `QMimeData`.
3. **Selection transforms: Interpolate / Transpose / Amplify** — *S, saves
   2–5 min on filter-sweep songs.* Three menu items on the current selection;
   interpolate is the high-value one for SID filter cutoff automation.
4. **Command palette (`Ctrl+P`)** — *S, saves 1–3 min by removing menu-
   hunting.* `QLineEdit` + `QSortFilterProxyModel` over a central `QAction`
   registry; every action also gets a discoverable shortcut entry.
5. **Cross-song instrument library dock** — *M, saves 5–15 min/new song.*
   `QFileSystemWatcher` over a user folder, list with ADSR thumbnails,
   drag-drop onto `InstrumentQuickList`, preview-on-select.
6. **Per-channel oscilloscope strip with click-to-mute** — *S, saves 1–2 min
   in mixing.* Promote the existing `scope_` ring buffer to a full-width
   strip; channel header click toggles mute, double-click solos.
7. **Performance bar (mute/solo, live tempo, master vol, edit lock)** — *S,
   safety win for live use.* One compact `QToolBar`; the edit lock prevents
   accidental edits during a demoparty performance.
8. **MIDI chord-input + velocity mapping** — *M, saves 2–5 min on chord-heavy
   writing.* Extend the jam path with accumulate-until-release and optional
   velocity→volume mapping. MIDI preferences dialog backed by `QSettings`.
9. **Read-only piano roll view (`PianoRollView`)** — *L, saves 2–4 min when
   reviewing or transcribing.* `QGraphicsView` rendering note-on/off runs
   from existing song data; click a bar to scroll `PatternView`. Edit
   support is a v2 conversation.
10. **Wavetable presets + hex paste/copy in `TablesView`** — *S, saves 3–10
    min/new instrument.* `QMenu` with built-in shapes and clipboard hex
    round-trip on the wave/pulse/filter tables.
11. **Song templates + 4 shipped templates** — *S, saves 1–2 min/new song.*
    `.sng` files in `examples/templates/`, populated from the directory at
    startup.
12. **Shortcut reference window + dockable layout save/restore** — *S,
    onboarding win.* Auto-generate reference HTML from the same command
    table that backs the palette; `QMainWindow::saveState`/`restoreState`
    persists dock layouts per named preset.

---

## Sources

- [Renoise Pattern Editor manual](https://tutorials.renoise.com/wiki/Pattern_Editor)
- [Renoise Pattern Matrix manual](https://tutorials.renoise.com/wiki/Pattern_Matrix)
- [Renoise Pattern Sequencer manual](https://tutorials.renoise.com/wiki/Pattern_Sequencer)
- [Renoise Graphical Automation manual](https://tutorials.renoise.com/wiki/Graphical_Automation)
- [Renoise Advanced Edit manual](https://tutorials.renoise.com/wiki/Advanced_Edit)
- [Renoise Keyboard Shortcuts manual](https://tutorials.renoise.com/wiki/Keyboard_Shortcuts)
- [Renoise 3.5 release news (Sonicstate)](https://sonicstate.com/news/2025/07/08/renoise-35-released/)
- [Renoise command_palette tool (forum)](https://forum.renoise.com/t/new-tool-3-4-command-palette/65543)
- [OpenMPT Manual: Patterns](https://wiki.openmpt.org/Manual:_Patterns)
- [OpenMPT Manual: Keyboard Actions](https://wiki.openmpt.org/Manual:_Keyboard_Actions)
- [OpenMPT Manual: Setup/Keyboard](https://wiki.openmpt.org/Manual:_Setup/Keyboard)
- [OpenMPT 1.32 release notes](https://openmpt.org/openmpt-1-32-01-00-released)
- [OpenMPT Features](https://openmpt.org/features)
- [MilkyTracker Manual](https://milkytracker.org/docs/manual/MilkyTracker.html)
- [MilkyTracker quick reference PDF](https://milkytracker.org/docs/QuickReferenceFT2WinSDL.pdf)
- [FamiStudio Documentation home](https://famistudio.org/doc/)
- [FamiStudio Editing Notes (piano roll)](https://famistudio.org/doc/pianoroll/)
- [FamiStudio Editing Patterns (sequencer)](https://famistudio.org/doc/sequencer/)
- [FamiStudio Basics](https://famistudio.org/doc/basics/)
- [Furnace manual PDF](https://tildearrow.org/furnace/doc/latest/manual.pdf)
- [Furnace quickstart](https://github.com/tildearrow/furnace/blob/master/doc/1-intro/quickstart.md)
- [Furnace FAQ (piano roll stance)](https://github.com/tildearrow/furnace/blob/master/doc/1-intro/faq.md)
- [Furnace oscilloscope docs](https://github.com/tildearrow/furnace/blob/master/doc/8-advanced/osc.md)
- [Furnace SID2 instrument docs](https://github.com/tildearrow/furnace/blob/master/doc/4-instrument/sid2.md)
- [DefleMask manual PDF](https://www.deflemask.com/manual.pdf)
- [BambooTracker wiki home](https://github.com/BambooTracker/BambooTracker/wiki)
- [BambooTracker configuration](https://github.com/BambooTracker/BambooTracker/wiki/Configuration)
- [BambooTracker FILEIO doc](https://github.com/BambooTracker/BambooTracker/blob/master/FILEIO.md)
- [Polyend Seq manual](https://polyend.com/manuals/seq-manual/)
- [Polyend Tracker product page](https://polyend.com/tracker/)
- [GoatTracker 2 (leafo fork)](https://github.com/leafo/goattracker2)
- [GTUltra (extended fork)](https://github.com/jpage8580/GTUltra)
- [ChiptuneSAK GoatTracker docs](https://chiptunesak.readthedocs.io/en/latest/goattracker.html)
- [Memento pattern overview (Refactoring.guru)](https://refactoring.guru/design-patterns/memento)
- [Memento pattern (Wikipedia)](https://en.wikipedia.org/wiki/Memento_pattern)
