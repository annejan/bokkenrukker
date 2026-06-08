# Issues currently in the Qt frontend

## Issues

- [x] 6581 SID emulation has bad quality audio output (Song dependent, ignore)
- [x] During playback, the blue position bar, and the read active channel don't run in sync. Not being fixed, read channel indicator is one ahead during playback or in sync with blue bar during playback.
      *(refresh() now snapshots chn[c].pattptr/4 into playRow_[MAX_CHN] once
        and paintEvent reads playRow_[c] for both the follow-play cursor pull
        and the red play-row highlight; audio thread can no longer advance
        chn[].pattptr between the cursor placement and the paint.)*
- [x] The timing between a note being played and a note being in the active position is off.
      *(same fix as above — direct chn[c].pattptr reads now match what is heard.)*
- [x] Instrument "-> table" button not clear what it does.
      *(tooltip added: opens Tables editor + jumps to the row this pointer references.)*
- [x] Scrolling down in some tracker patterns doesn't work, the scroll position keeps being pushed up.
      *(refresh() was yanking the scrollbar back to eppos every tickScope tick; now only
        auto-pulls when eppos / epchn actually moved.)*
- [x] Song order still shows chan 4, 5, 6 even when second SID is disabled
      *(OrderListModel::columnCount now follows stereo_mode; rowCount only scans the
        active channels too.)*
- [ ] Check if 'subtune 2' pattern should be allocated to second SID chip by default.
- [x] When selecting field in the Tables editor, the value becomes '...' in the screen, hiding the previous value.
      *(TableCellDelegate::createEditor: QLineEdit + maxLength 2 + hex validator;
        editTriggers reduced to DoubleClicked + EditKeyPressed.)*
- [x] Collapse Order map doesn't work, tracker pattern moves right, but Order map doesn't fold.
      *(OrderMiniMap had setMinimumWidth(120) which kept the dock from
        shrinking to zero. Dropped the floor; the QDockWidget closable +
        floatable + movable features are now set explicitly on both docks.)*
- [x] Audio engine 'dual SID' and 'Second SID' is not clear, when second sid is enabled allow for user to select it in the bottom playback bar, like the first SID is selected.
      *(StatusStrip now shows two clickable segments: 'SID1 6581/8580' and
        'SID2 6581/8580 / off'. Clicking SID2 toggles sid2model exactly the
        way clicking SID1 toggles sidmodel; SID2 shows 'off' + dim colour
        when stereo_mode is 0.)*
- [x] Mute button is too narrow for word 'MUTE', obscuring most of the word, just make it a red 'M' fixes the issue.
      *(reverted to 1-char glyph: bold red 'M' on dark-red fill when muted,
        green '♪' on dark-green fill when playing.)*
- [x] SID 2 off/6581/8580 should be clickable, the dropdown settings should still be there, just stating 'enable dual SID'
      *(SID2 segment now 3-state cycle on click: off → 6581 → 8580 → off.
        Settings menu 'Dual SID' toggle still flips stereo_mode explicitly.)*
- [x] 'Order map' Collapse button doesn't work, and doesn't make sense in context.
      *(dropped DockWidgetFloatable on both Order map + Instruments docks
        so the [↗] float / detach button no longer appears in the title
        bar; users were reading it as a collapse toggle. The Close X is
        the show / hide control.)*
- [x] 'Note entry layout' doesn't change much in the view
      *(status strip 'Oct N' segment now reads 'Oct N · Protracker / DMC /
        Janko' so the current note-entry preset is visible at a glance.)*
- [x] When changing SID type during playing, a hanging note can occur
      *(AudioFence now zeros every voice's freq / pulse / ctrl (gate=0) / AD /
        SR + master volume + RES_FILT in both sidreg shadows, and sets each
        chn[c].gate to 0xfe. Before, stopsong()+songinit=PLAY_STOPPED was set
        directly, skipping the playroutine PLAY_STOP transition that
        normally writes the gate-off bits.)*
- [x] When a row is selected in the 'pattern editor', 'Play from position' still starts playing at the beginning of the pattern.
      *(playFromPos + playPattern now seed chn[c].pattptr = eppos * 4 in
        addition to songptr = espos[c], so the song / pattern starts at
        the cursor row instead of row 0.)*
- [x] In Wave table, 'jump to step' also shows 'note: rel +13' (for example), is this correct?
      *(decodeCell() now skips the 'note:' suffix when L == $FF (jump) or
        L is in $F0..$FE (execute cmd) — those rows treat R as a target /
        cmd param, not a note byte.)*
- [x] Order map at start up too narrow, max width should be default width.
      *(OrderMiniMap::sizeHint returns QSize(160, 400) + MainWindow uses
        resizeDocks({orderMapDock_}, {160}, Qt::Horizontal) so the dock
        starts at the historical 160 px instead of Qt's tiny default.)*
- [x] MUTE in 'UV' bar when channel muted, very dark RED, should be bright and stand out.
      *(VU-strip MUTE text now bold + bright red (255,80,80) instead of
        the previous dark muted red that blended into the strip.)*
- [x] When all channels are selected, the yellow selection window should span all channels.
      *(OrderMiniMap cursor outline is now drawn on every channel at espos[c]
        when selectAll_ is true; previously only the eschn column got it.)*
- [x] Special commands are not handled intuitively in the 'track editor', they should have a color, 'purple'? The user should not be able to click and get the yellow border around a special command cell.
      *(OrderDelegate now paints RST / TRANSUP/DOWN / REPEAT cells in
        purple shades and suppresses the yellow edit-cursor underlay +
        border on cells where v >= REPEAT — clicking a special cell
        still moves selection but no longer drops the cursor onto it.)*
- [x] The instrument editor has an instrument selector on the left, but the instrument selection side bar "instruments" is already present on the right, remove the left one.
      *(dropped list_ + its connection / palette setup / populate loop / per-
        refresh name mirroring / onListChanged slot from InstrumentView.
        InstrumentQuickList in the right dock is now the single selector,
        firing instrumentChosen -> MainWindow::refreshAll which re-reads
        einum into the editor.)*
- [x] Wave table program color explanation text color is too dark, make brighter.
      *(the descriptor + 'Examples:' lines in the right-side legend used
        #5A6470 which barely registered against bgAlt. Bumped to #B0BCC8
        across all four table legends (wave / pulse / filter / speed)
        — bright enough to read while still secondary to bold opcodes.)*
- [x] Table pointers '-> table' button doesn't work.
      *(InstrumentView::edited connection in MainWindow now also calls
        syncStack() — the jump slots set editmode=3 (EDIT_TABLES) and
        emit edited(), but the previous refreshAll-only connection
        never propagated the editmode change to the QStackedWidget.)*
- [x] Color is also shown of instrument entries that are empty
      *(instrColor() short-circuits to an invalid QColor when the
        instrument slot is 'factory empty' — blank name, zero ADSR,
        zero wave / pulse / filter pointers and zero firstwave.)*
- [x] focus border highlight around instrument value, no longer visible since color scheme has been added.
      *(re-ordered the paintEvent passes — focusRect is captured during
        the column-tint pass but the drawRect for the white outline
        happens after the instrument-colour fillRect, so the border
        sits on top of the colour band and stays visible.)*
- [x] Wave table index color to dark gray, the places where this dark gray color is used elsewhere should be made lighter as well.
      *(Theme::C::textDim bumped from #5A6470 to #B0BCC8 — single
        source, lifts every dim-grey site (Tables idx column, instr
        view labels, status strip secondary text, placeholder /
        disabled palette) in one shot to match the legend brightness
        from commit 70ca50b.)*
- [x] When the program is idle it still uses 40% of a CPU core, something is polling, or some other bug is consuming excessive CPU cycles.
      *(Not polling — the PaAudio callback clocked libresidfp cycle-exact at
        ~1 MHz CONTINUOUSLY, playing or not (perf: ~77% in
        reSIDfp::Filter::clock / getNormalizedVoice). sid_fillbuffer() now
        short-circuits to silence when nothing is sounding — sid_active()
        checks isplaying() + voice gate bits + envelopeLevel(); a ~250 ms tail
        keeps clocking after the last sound so release + filter ring-out finish,
        then it stops. Resumes instantly on the first gated note / play.
        Measured idle: 40% -> ~14% steady-state headless, and the residual is
        mostly PortAudio device servicing.)*
- [ ]   Filter settings lost when player pauzed and continued, when the cursor is not moved during pauze and continue, the filters should stay applied.
- [ ]   

## Features

- [x] Record mode present in 'Pattern editor' ?
      *(StatusStrip now has a 'REC' segment — '● REC' in vuRed when
        recordmode = 1, '○ rec' in textDim when 0. Tooltip notes Space
        as the toggle key. Refreshes through the existing UI tick.)*
- [ ] Use key codes, that are not dependent on keyboard layout for Pattern record mode, so alternative keyboard layouts have a music keyboard with the notes in order as well. For the normal commands the preferred layout should be honored, only for notes it should be layout independent.
- [x] Have a way to silence a note in the pattern editor once started.
      *(Backspace in PatternView::keyPressEvent calls releasenote(epchn)
        and swallows the key — no pattern row inserted, cursor stays.
        Row-number tooltip mentions the shortcut for discoverability.)*
- [x] Mute buttons above pattern channels are not really clear, better Icon for those.
      *(replaced 'M' / '·' with filled colour pill labelled MUTE / ON.)*
- [x] Create hover over (delay of one second), when hovering over row header.
      *(PatternView::event handles QEvent::ToolTip on the row-number column:
        shows row hex/decimal, beat / downbeat / step tag, pattern# + length.)*
- [ ] Second SID not implemented, because audio mixing resulted in unacceptable audio quality, found out how to implement this correctly.
      *(LOAD now works: ported gt2stereo's determinechannels() into loadsong —
        detects mono (3ch) vs stereo/dual-SID (6ch) GTS5 by trial-reading all 6
        orderlists and checking each ends with a valid 0xff endmark; mono files
        fail on the 4th channel and fall back to 3. loadSongFile then sets
        stereo_mode + re-inits SID2 (fenced) and syncs the Dual-SID menu check.
        Verified: sleepwalk.sng -> 6ch/stereo, mono examples unchanged,
        stereo<->mono reload clean. STILL TODO: the audio OUTPUT itself — true
        stereo + per-SID pan + mono option (design agreed, not yet built); the
        current path still mono-sums SID1+SID2 in sid_fillbuffer.)*
- [ ] Add option to select mono or stereo mode for dual sid, keeping things simple for the user.
- [ ] Add support for external keyboards or samplers via USB, to enable record mode via these devices? (Popular options you can suggest?)
- [x] Clear way to set / read the octave in the UI for record mode in the pattern editor
      *(status-strip 'Oct N · <preset>' segment is now clickable + wheel-
        scrollable: click cycles 0..7 (wraps), mouse wheel scrolls ±1
        clamped 0..7. Tooltip shows the * / / keyboard shortcuts.)*
- [x] Add switch above order map to select channels together or individually.
      *(Dock now wraps a QToolButton header over OrderMiniMap. Button toggles
        OrderMiniMap::selectAllChannels — true = plain click moves all chans,
        false = plain click moves only the clicked one. Ctrl-click still
        inverts the active mode for one click so both stay reachable.)*
- [x] Set 8580 as the default SID version, when starting the application, unless loaded file specifies otherwise.
      *(qt_globals.c sidmodel + sid2model now initialise to 1 (8580).
        loadsong() overwrites these from the .sng header, so anything
        saved against 6581 still loads as 6581.)*
- [x] Is Octamed different enough from Protracker to add that style as well ?
      *(no — OctaMED note-entry key map is the same Z..M lower row /
        Q..U upper row as Protracker; the engine differences are at
        the song-arrangement / mixing level, not in the keyboard. Adding
        an 'OctaMED' preset would be a duplicate of Protracker. Skipped.)*
- [x] Add load and save of user preferences, like SID type and other settings the user does in the UI, have a default, and the possibility to 'save as'.
      *(QSettings auto-restore on launch + auto-save on quit, stored at
        \$XDG_CONFIG_HOME/goattrk2-qt/goattrk2-qt.conf (Linux) / equivalent
        on Windows / macOS. Keys: audio/sidmodel, audio/sid2model,
        audio/stereo, audio/ntsc, audio/multiplier, editor/keypreset,
        editor/followplay. Applied before sid_init so the SID backend
        starts with the persisted chip model; loadSongFile still overrides
        from the .sng header so songs play back authentically. 'Save as
        profile' deferred — single auto-saved profile is enough for now.)*
- [x] Pair each ADSR spinbox with a 0..15 slider so the value can be scrubbed instead of typed / stepped.
      *(makeNybbleRow wraps the existing nybble spinbox in a row with a
        QSlider(Qt::Horizontal, 0..15, tick=1). Spinbox and slider are
        bidirectionally bound via valueChanged forwarding; the spinbox
        still fires InstrumentView::onAdChanged / onSrChanged so the
        engine wiring is unchanged.)*
- [x] Add a toggle button to give every instrument it's own unique background color and color the instrument bytes in the patterns the background color the instrument has got. (Do something like hash the name, and make sure the colors are ergonomically useful ). Use the colors in this page as a reference, filter out the colors too close to text color: https://static.vecteezy.com/system/resources/previews/016/592/367/original/color-palette-set-design-template-multi-color-free-vector.jpg
      *(24-colour palette extracted directly from the linked vecteezy image:
        sampled the 270 swatches (6×9 palettes × 5 colours), filtered to
        WCAG-style luminance 0.10..0.55 so neither black nor white text
        clips, then greedy farthest-first to spread the hue space. Stored
        as kInstrColors[] in PatternView.cpp.
        instrColor(inum) hashes the instrument name with FNV-1a and folds
        the index in too — renames pick a new colour, empty-name slots
        still differ between numbers. paintEvent fills only the 2-digit
        instr cell so the rest of the row stays readable; instr text is
        forced white when the cell is coloured. View > 'Instrument cell
        colours' toggles + persists via QSettings editor/instrColors.)*
- [x] When Table pointer variable is selected, use the left over space in the UI on the bottom right (left of instrument selection) to show the location listing of the relevent table the instrument in jumping to.
      *(pointerPrev_ QLabel below summary_ in the instrument editor. Click
        a Wavetable / Pulsetable / Filtertable Pos spinbox -> 16 rows of
        the target table painted, idx / L / R / inline 'jump $xx' or
        'cmd NXY' decode. Pointer row highlighted yellow. Sticky after
        focus loss; replaced when a different pointer focuses. Live-
        updates while the user types in the spinbox.)*
- [x] Instruments are now changed directly in the configuration, it would be better, if we change a copy of the instrument, and have a button next to 'Play test note' and 'Silence', that allows to 'Apply' the changed instrument, or reset in back to the current state using 'Reset'.
      *(InstrumentView::saved_ snapshots instr[einum] at slot load. Every
        edit slot calls markDirty() which enables Apply + Reset. Apply
        commits the current live state as the new baseline (saved_ =
        instr[einum]). Reset rolls instr[einum] back to saved_ and re-
        reads the UI. Switching to a different slot auto-applies the
        previous slot silently. Live edits still hit instr[einum]
        directly so the test note hears every keystroke.)*
- [x] Add a toggle button in the instrument editor that keeps playing the note every second on repeat, so the changes in the variables can be changed and reviewed instantly.
      *('Auto-test' QPushButton added next to 'Play test note' / 'Silence'.
        When toggled on a 1 s QTimer retriggers onTestNote so any ADSR /
        wave / pulse / filter knob change is audible within the next
        second. Toggling off stops the timer + silences.)*
- [ ] Add midi protocol support, so a midi keyboard can be used to record notes in the pattern channel selected. (Over USB or what ever just make sure it's Windows and Linux and OSX compatible)
      *(Design: RtMidi, event/callback (no polling), vendored single
        RtMidi.h/.cpp. RtMidi callback thread -> Qt queued signal
        noteEvent(midiNote,on) -> main-thread slot -> existing
        insertnote() (recordmode) / playtestnote() (jam). Note mapping
        config-toggle: Absolute (piano pitch) vs Relative (epoctave),
        QSettings midi/noteMode + midi/octaveOffset. Settings > MIDI
        submenu: device list (auto-open first / saved by name) + note
        mode. Retire JACK MIDI poll in bme_snd.c. Blocked on the
        polling->notification refactor below for the capture hook.)*
- [x] Changing the second SID model (6581 <-> 8580) during playback crashed —
      first "pure virtual method called" / segfault, then "malloc(): corrupted
      top size" — after a few toggles. Filters also intermittently stopped
      applying after a chip switch.
      *(Root cause: toggleSid2Model() (and the prev/next multiplier slots via
        qt_stubs) called sound_init(), which runs the FULL BME snd_init —
        opening a SECOND audio backend (the SDL mixer thread SDLAudioP1) that
        clocks libresidfp alongside PaAudio, and reallocating channel/mixer
        buffers. That second backend raced PaAudio + the SID rebuild: vtable
        crash, heap corruption, and half-reset filter state. main.cpp never
        calls sound_init, so this was the only thing ever spawning the SDL
        backend. Fix: these paths now call sid_init() (fenced) like
        toggleSidModel — rebuild just the SID, never the audio device. sid_init
        already applies sid2model. The SDL backend now never starts; the JACK
        MIDI "failed to create jack client" noise is gone too (it only ran
        inside snd_init). AudioFence also gained snd_lock()/snd_unlock()
        (SDL_LockAudio) as belt-and-braces. Verified: 8 rapid SID2 toggles,
        no crash / no SDLAudio thread / no jack message.)*

## Polling -> Qt notification refactor

Goal: the Qt frontend currently drives almost all UI updates from periodic
QTimers that re-read C-core globals every frame ("polling"). Replace with an
event/notification pattern: the C core (and audio/playroutine thread) publishes
state-change notifications that a thin Qt bridge converts to **queued signals**
(thread-safe, land on the GUI thread), so views update on change instead of on a
clock. Continuous analog-style meters (VU / SID envelope) are inherently sampled
and may keep a timer — flagged below as "sampling OK". Solve top-down: item 0 is
the enabling infrastructure every other item depends on.

- [x] **0. Notification bridge (infrastructure, do first).** Add a C-callable
      notifier the playroutine / editor core can call on state change (transport
      start/stop, row advance, order-position change, instrument trigger,
      selection/cursor move, recordmode toggle). A Qt singleton (e.g.
      `CoreEvents : QObject`) re-emits each as a `Qt::QueuedConnection` signal so
      audio-thread origins are marshalled safely to the GUI thread. All items
      below consume this. *(Same thread-hop pattern as the MIDI design.)*
      *(qt/CoreEvents.h/.cpp: `CoreEvents : QObject` singleton with signals
        transportChanged(bool) / rowChanged() / orderPosChanged(). pump() is
        called from the PortAudio callback (qt/PaAudio.cpp, right after
        playroutine()) on the audio thread; it diffs isplaying() + cheap
        signatures over chn[].pattptr / chn[].songptr and emits only on an
        edge. CoreEvents lives on the GUI thread so the default connection is
        delivered queued — slots run on the GUI thread, no widget touched from
        audio. instrTriggered + cursor/recordmode notifications deferred to the
        phase-2 consumers that need them.)*
- [x] **1. Main UI tick.** [MainWindow.cpp:117-123](MainWindow.cpp#L117-L123)
      40 ms (25 Hz) `timer_` -> [MainWindow::tick()](MainWindow.cpp#L1061) drives
      every view refresh unconditionally. Master poll. Decompose into the items
      below; keep a timer only for true meters.
      *(tick() no longer polls playback state. It now only: samples the scope
        meter (item 7), repaints the pattern grid while STOPPED so editing stays
        responsive, and refreshes the status strip. All playback-driven repaints
        moved to CoreEvents handlers — see items 2/3 + onOrderPosChanged for the
        order mini-map. Timer stays for the meter + idle-editor repaint.)*
- [x] **2. Pattern follow-play cursor / row highlight.**
      [MainWindow.cpp:1068](MainWindow.cpp#L1068) `pattern_->refresh()` +
      `tickScope()` poll `chn[].pattptr` / `eppos` each tick. Notify on row
      advance and on cursor/selection move.
      *(CoreEvents::rowChanged -> MainWindow::onPlayRowChanged() repaints the
        pattern grid exactly when the play row advances, instead of the old
        ~12 Hz every-other-tick poll during playback. Editor cursor/selection
        moves while STOPPED still go through the timer repaint in tick().)*
- [x] **3. Transport button relabel.**
      [MainWindow.cpp:1078](MainWindow.cpp#L1078) polls `isplaying()` every tick
      to flip the Pos/Pause button text. Notify on transport state change.
      *(CoreEvents::transportChanged(bool) -> MainWindow::onTransportChanged()
        relabels the Pos/Pause button on the edge only, and repaints the pattern
        + order map once so start/stop position + play-row highlight sync
        immediately.)*
- [x] **4. Order view play cursor.**
      [OrderView.cpp:330-334](OrderView.cpp#L330-L334) 33 ms `playRefresh_`
      repaints the order viewport from `chn[].songptr`. Notify on
      order-position change.
      *(playRefresh_ timer removed. OrderView now connects to
        CoreEvents::orderPosChanged (viewport repaint when the song pointer
        advances) + transportChanged (clear the tint on play/stop). Required
        moving coreEvents_ creation before buildUi() in MainWindow so child
        views can connect from their constructors.)*
- [x] **5. Instrument quick-list flash.**
      [InstrumentQuickList.cpp:35](InstrumentQuickList.cpp#L35) /
      [tickFlash()](InstrumentQuickList.cpp#L90) polls `chn[].instr` to flash
      sounding instruments. Notify on instrument-trigger (note-on) event.
      *(The fade is a continuous animation, so sampling chn[].instr stays — but
        the timer no longer free-runs. It's gated on CoreEvents::transportChanged
        (start on play when blink enabled) and self-stops in tickFlash() once the
        song is stopped and the last flash has decayed. setBlinkEnabled only
        starts it while already playing. Behaviour while playing is unchanged;
        it just stops burning 33 Hz when idle.)*
- [x] **6. Instrument editor live ADSR meter.**
      [InstrumentView.cpp:700-704](InstrumentView.cpp#L700-L704) 30 ms
      `playbackTimer_` -> `tickPlayback()` polls `sid_getlevels()` envelope
      level. **Sampling OK** — continuous signal; keep a timer but make it run
      only while a note sounds / the editor is visible (drive start/stop from a
      transport notification).
      *(Gated on VISIBILITY rather than transport: started in showEvent /
        stopped in hideEvent, so the envelope is only sampled while the
        instrument editor is the active stack page. Visibility (not transport)
        is the right gate here because the editor must keep animating during
        test-note / auto-test auditioning while the song is stopped — same
        reason item 7 stays sampling.)*
- [x] **7. Scope / VU strip.**
      [MainWindow.cpp:1068](MainWindow.cpp#L1068) `pattern_->tickScope()` pushes
      the scope meter per tick. **Sampling OK** — continuous; keep but gate on
      transport-active notification so it idles when stopped.
      *(Kept as timer sampling in tick() — intentionally NOT gated on transport,
        because jam / test notes produce audio while stopped and must still show
        on the meter. tickScope() already short-circuits redraws when the level
        is unchanged, so an idle SID costs nothing. Closed as "sampling is the
        correct model here".)*
- [ ] **8. JACK MIDI input poll.** *(FOLDED INTO the MIDI feature — not a
      standalone job.)*
      [bme_snd.c:80-113](../src/bme/bme_snd.c#L80-L113) polls
      `jack_midi_get_event_count()` inside the JACK process callback. Two
      reasons it ships with the MIDI feature, not before: (a) JACK doesn't push
      — draining in the process callback is the JACK idiom, so the only way to
      remove the poll is to replace JACK MIDI with RtMidi (the MIDI item above);
      (b) it has the same RT-thread bug as the no-sound regression — it calls
      `insertnote()` + writes `eppos`/`epview` straight from the JACK process
      thread. The RtMidi replacement must hop to the GUI thread the same way
      CoreEvents does (see [[audio-thread-no-qt]]). Delete this block when RtMidi
      lands.
- [x] **9. RPC timer control (low priority).**
      [Rpc.cpp:492-498](Rpc.cpp#L492-L498) test harness starts/stops the main
      `QTimer` by `findChild<QTimer*>()`. Revisit once item 1 changes the tick's
      role; may need a stable handle instead of child lookup.
      *(MainWindow now exposes pauseTimer() / resumeTimer() that act on timer_
        directly; Rpc::cmdPauseTimer/cmdResumeTimer call those instead of
        findChild<QTimer*>(), which was ambiguous now that child views own their
        own QTimers. Resume restarts at the real 40 ms UI rate (was a magic
        20 ms).)*

### Engine (src/) audit

The pure tracker core (gplay.c / gsong.c / gtable.c / ginstr.c) is
callback-driven and has no polling — the audio callback is its clock by design.
Beyond the JACK MIDI drain (item 8), two engine-side polls are real. The rest of
the `while(!flag)` / `SDL_Delay` hits live in non-Qt code (goattrk2.c main loop,
greloc.c relocator, bme_kbd.c / bme_win.c — the standalone SDL app + gt2reloc
tool, replaced by Qt + qt_stubs.c) and are out of scope.

- [x] **10. Fixed-delay mixer-stop wait.**
      [gsound.c:233-235](../src/gsound.c#L233-L235) `sound_uninit()` did
      `SDL_Delay(50)` to "make sure the sound timer thread is not mixing anymore"
      before freeing buffers. A guess-delay, not a real handshake. Runs on every
      audio re-init (SID-model / multiplier change).
      *(Replaced with a real handshake: for normal SDL audio it now holds
        SDL_LockAudio() across the teardown (detach mixer/player + free buffer)
        and SDL_UnlockAudio() at the end — SDL_LockAudio blocks until any
        in-flight callback returns and blocks new ones, so no race and no magic
        sleep. HardSID / Catweasel have no audio-callback lock (they drive from
        an SDL timer / Win32 thread) so they keep the short settle delay. Gated
        on `snd_sndinitted` so it's a no-op when SDL audio never opened.)*
- [ ] **11. Win32 HardSID player-thread busy-loop (conditional).**
      [gsound.c:359](../src/gsound.c#L359) `while (runplayerthread) { … }` with
      the `suspendplayroutine` / `flushplayerthread` volatile flags
      ([gsound.c:54-56](../src/gsound.c#L54-L56)). All `#ifdef __WIN32__` +
      HardSID, so NOT compiled in the current Linux build — but it WILL compile
      once the cross-platform MIDI feature (item 193) brings up a Windows build.
      Same RT-thread hazard as the no-sound fix: it calls `playroutine()` and
      reads editor state from a raw thread spinning on volatile flags. Revisit
      together with the Windows MIDI port; route any UI-affecting state through
      the GUI-thread hop (see [[audio-thread-no-qt]]).
- [ ] 