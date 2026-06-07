#pragma once
#include <QColor>

// Stable per-instrument background colour. Palette is 24 colours sampled
// from the vecteezy reference image, filtered to luminance 0.10..0.55,
// farthest-first deduped. The colour for an instrument is derived from a
// FNV-1a hash of its name + slot index.
//
// Opt-in per instrument: instrColor() returns an invalid QColor unless
// setInstrColored(i, true) has been called for that slot. This keeps the
// editor from becoming a wall of colour by default — the user picks the
// instruments they want to track visually.
QColor instrColor(unsigned char inum);

bool isInstrColored(unsigned char inum);
void setInstrColored(unsigned char inum, bool on);
void setAllInstrColored(bool on);     // master 'all on' / 'all off'

// QSettings round-trip. Stored as a 64-bit mask under editor/instrColorMask.
void loadInstrColorMask();
void saveInstrColorMask();
