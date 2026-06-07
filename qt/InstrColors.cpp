#include "InstrColors.h"

#include <QSettings>

extern "C" {
#include "gcommon.h"
extern INSTR instr[MAX_INSTR];
}

#include <cstdint>

// One bit per slot (MAX_INSTR=64). User toggles via double-click on the
// right-side instrument list; persisted as editor/instrColorMask in
// QSettings.
static uint64_t s_colorMask = 0;

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

// Empty instrument = all-zero name + no ADSR / wave / pulse / filter
// pointers + no firstwave. Those slots are factory defaults and
// shouldn't get coloured even if their mask bit is set (e.g. via the
// 'all on' master button) — otherwise the dock fills with palette tiles
// for instruments the user never even touched.
static bool instrIsEmpty(unsigned char inum) {
    if (inum == 0 || inum >= MAX_INSTR) return true;
    const INSTR &x = instr[inum];
    for (int i = 0; i < MAX_INSTRNAMELEN; i++)
        if (x.name[i] != 0 && x.name[i] != ' ') return false;
    if (x.ad || x.sr) return false;
    if (x.ptr[WTBL] || x.ptr[PTBL] || x.ptr[FTBL]) return false;
    if (x.firstwave) return false;
    return true;
}

QColor instrColor(unsigned char inum) {
    if (inum == 0 || inum >= MAX_INSTR) return QColor();
    if (!(s_colorMask & (1ull << inum))) return QColor();
    if (instrIsEmpty(inum)) return QColor();
    uint32_t h = 2166136261u;
    for (int i = 0; i < MAX_INSTRNAMELEN; i++) {
        unsigned char ch = (unsigned char)instr[inum].name[i];
        if (ch == 0) break;
        h ^= ch; h *= 16777619u;
    }
    h ^= inum; h *= 16777619u;
    return QColor::fromRgb(kInstrColors[h % kInstrColorCount]);
}

bool isInstrColored(unsigned char inum) {
    if (inum == 0 || inum >= MAX_INSTR) return false;
    return (s_colorMask & (1ull << inum)) != 0;
}

void setInstrColored(unsigned char inum, bool on) {
    if (inum == 0 || inum >= MAX_INSTR) return;
    if (on) s_colorMask |=  (1ull << inum);
    else    s_colorMask &= ~(1ull << inum);
}

void setAllInstrColored(bool on) {
    if (on) {
        // Bits 1..MAX_INSTR-1 (slot 0 is the 'no instrument' sentinel).
        s_colorMask = (MAX_INSTR >= 64) ? ~0ull
                                       : (((1ull << MAX_INSTR) - 1) & ~1ull);
        // Clear bit 0 — never colour the empty slot.
        s_colorMask &= ~1ull;
    } else {
        s_colorMask = 0;
    }
}

void loadInstrColorMask() {
    QSettings s;
    s_colorMask = s.value("editor/instrColorMask", (qulonglong)0).toULongLong();
}

void saveInstrColorMask() {
    QSettings s;
    s.setValue("editor/instrColorMask", (qulonglong)s_colorMask);
}
