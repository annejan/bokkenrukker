#pragma once
#include <QColor>

// Stable per-instrument background colour. Palette is 24 colours sampled
// from the vecteezy reference image, filtered to luminance 0.10..0.55,
// farthest-first deduped. The colour for an instrument is derived from a
// FNV-1a hash of its name + slot index — renames pick a fresh colour,
// empty-name slots still differ between numbers.
//
// Returns an invalid QColor for instrument 0 / out-of-range so callers
// can check QColor::isValid() before painting.
QColor instrColor(unsigned char inum);
