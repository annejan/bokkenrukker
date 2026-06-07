# Issues currently in the Qt frontend

## Issues

- [ ] 6581 SID emulation has bad quality audio output
- [ ] During playback, the blue position bar, and the read active channel don't run in sync. Not being fixed, read channel indicator is one ahead during playback or in sync with blue bar during playback.
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
- [ ] Collapse Order map doesn't work, tracker pattern moves right, but Order map doesn't fold.
- [ ] Audio engine 'dual SID' and 'Second SID' is not clear, when second sid is enabled allow for user to select it in the bottom playback bar, like the first SID is selected.
- [ ] Mute button

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
