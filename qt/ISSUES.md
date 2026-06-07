# Issues currently in the Qt frontend

## Issues

- [ ] 6581 SID emulation has bad quality audio output
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
- [ ] Check if subtune 2 should be allocated to second SID chip by default.
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
- [ ] Second SID results in segmentation fault at the moment.
- [ ] Collapse button doesn't work, and doesn't make sense in context.
- [ ] 'Note entry layout' doesn't change much in the view
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
- [ ] 

## Features

- [ ] Make the ADSR share UI component interactive, so dragging becomes possible
- [ ] Show moving bar in ADSR UI component where the sample is (If possible, and frame drops are allowed).
- [ ] Highlight wave table lines that are active in the same way the instruments are highlighted, filter for visible, and only update view when, a delta is present and the element is visible.
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
