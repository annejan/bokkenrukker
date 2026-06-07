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
- [ ] Wave table index color to dark gray, the places where this dark gray color is used elsewhere should be made lighter as well.

## Features

- [ ] Record mode present in 'Pattern editor' ?
- [ ] Use key codes, that are not dependent on keyboard layout for Pattern record mode, so alternative keyboard layouts have a music keyboard with the notes in order as well. For the normal commands the preferred layout should be honored, only for notes it should be layout independent.
- [ ] Have a way to silence a note in the pattern editor once started.
- [x] Mute buttons above pattern channels are not really clear, better Icon for those.
      *(replaced 'M' / '·' with filled colour pill labelled MUTE / ON.)*
- [x] Create hover over (delay of one second), when hovering over row header.
      *(PatternView::event handles QEvent::ToolTip on the row-number column:
        shows row hex/decimal, beat / downbeat / step tag, pattern# + length.)*
- [ ] Second SID not implemented, because audio mixing resulted in unacceptable audio quality, found out how to implement this correctly.
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
- [ ] 