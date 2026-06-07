# Issues currently in the Qt frontend

## Issues

- 6581 SID emulation has bad quality audio output
- During playback, the blue position bar, and the read active channel don't run in sync
- The timing between a note being played and a note being in the active position is off.
- Instrument "-> table" button not clear what it does.
- Scrolling down in some tracker patterns doesn't work, the scroll position keeps being pushed up.
- Song order still shows chan 4, 5, 6 even when second SID is disabled
- Check if subtune 2 should be allocated to second SID chip by default.
- When selecting field in the Tables editor, the value becomes '...' in the screen, hiding the previous value.

## Features 

- Make the ADSR share UI component interactive, so dragging becomes possible
- Show moving bar in ADSR UI component where the sample is (If possible, and frame drops are allowed).
- Highlight wave table lines that are active in the same way the instruments are highlighted, filter for visible, and only update view when, a delta is present and the element is visible.
- Record mode present in 'Pattern editor' ? 
- Use key codes, that are not dependent on keyboard layout for Pattern record mode, so alternative keyboard layouts have a music keyboard with the notes in order as well. For the normal commands the preferred layout should be honored, only for notes it should be layout independent.
- Have a way to silence a note in the pattern editor once started.
- Mute buttons above pattern channels are not really clear, better Icon for those.
- Create hover over (delay of one second), when hovering over row header.
- Second SID not implemented, because audio mixing resulted in unacceptable audio quality, found out how to implement this correctly.