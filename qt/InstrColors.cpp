#include "InstrColors.h"

extern "C" {
#include "gcommon.h"
extern INSTR instr[MAX_INSTR];
}

#include <cstdint>

static const QRgb kInstrColors[] = {
    qRgb(242,  64,  61), qRgb( 45, 217, 197), qRgb( 57, 104,  36),
    qRgb(185, 160, 252), qRgb(225, 193,  73), qRgb( 14,  97, 163),
    qRgb(149, 104, 124), qRgb(253, 137, 174), qRgb(148, 166,   6),
    qRgb(120, 212, 149), qRgb(187,   7,  80), qRgb(231, 156,   5),
    qRgb( 64, 153, 147), qRgb(193, 173, 136), qRgb(125, 190, 234),
    qRgb(254, 123,  93), qRgb(123,  97,  62), qRgb(240,  22, 117),
    qRgb(218,  63,   2), qRgb(115, 112,   0), qRgb( 82, 162,  41),
    qRgb(  5, 164, 166), qRgb( 57, 110, 102), qRgb(199, 190, 194),
};
constexpr int kInstrColorCount =
    sizeof(kInstrColors) / sizeof(kInstrColors[0]);

QColor instrColor(unsigned char inum) {
    if (inum == 0 || inum >= MAX_INSTR) return QColor();
    uint32_t h = 2166136261u;
    for (int i = 0; i < MAX_INSTRNAMELEN; i++) {
        unsigned char ch = (unsigned char)instr[inum].name[i];
        if (ch == 0) break;
        h ^= ch; h *= 16777619u;
    }
    h ^= inum; h *= 16777619u;
    return QColor::fromRgb(kInstrColors[h % kInstrColorCount]);
}
