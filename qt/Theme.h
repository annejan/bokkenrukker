#pragma once
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QFontDatabase>
#include <QWidget>

namespace Theme {

inline QFont monoFont(int pt = 11) {
    QFont f(QStringList{"JetBrains Mono", "Iosevka Term", "IBM Plex Mono", "DejaVu Sans Mono"}, pt);
    f.setStyleHint(QFont::Monospace);
    f.setHintingPreference(QFont::PreferFullHinting);
    return f;
}

inline QFont uiFont(int pt = 10, bool bold = false) {
    QFont f;
    f.setPointSize(pt);
    f.setBold(bold);
    return f;
}

// Palette — desaturated C64 / Colodore inspired, tuned for dark UI.
namespace C {
inline const QColor bgBase     {0x14, 0x18, 0x1E};
inline const QColor bgAlt      {0x1A, 0x1F, 0x28};
inline const QColor beat       {0x1F, 0x2D, 0x44};
inline const QColor downbeat   {0x2D, 0x40, 0x60};
inline const QColor editRow    {0x3D, 0x5A, 0x8C};
inline const QColor playRow    {0x5A, 0x2E, 0x2E};
inline const QColor sep        {0x2A, 0x32, 0x3E};
inline const QColor text       {0xE0, 0xE6, 0xEE};
inline const QColor textDim    {0x5A, 0x64, 0x70};
inline const QColor note       {0xE0, 0xE6, 0xEE};
inline const QColor instr      {0x88, 0xC0, 0xD0};
inline const QColor cmdDigit   {0xD0, 0x87, 0x70};
inline const QColor cmdParam   {0xEB, 0xCB, 0x8B};
inline const QColor highlight  {0xFF, 0xD9, 0x3D};
inline const QColor rst        {0xE0, 0x7F, 0x4A};
inline const QColor repeatCol  {0xE0, 0xC8, 0x4A};
inline const QColor transpose  {0x88, 0xC0, 0xD0};
inline const QColor vuGreen    {0x3F, 0xB9, 0x50};
inline const QColor vuAmber    {0xD9, 0xA4, 0x41};
inline const QColor vuOrange   {0xE5, 0x72, 0x3B};
inline const QColor vuRed      {0xE5, 0x48, 0x4D};
inline const QColor vuBg       {0x0E, 0x11, 0x17};
inline const QColor peakTick   {0xF0, 0xF6, 0xFC};
}

inline void applyDarkPalette(QWidget *w) {
    w->setAutoFillBackground(true);
    QPalette pal = w->palette();
    pal.setColor(QPalette::Window, C::bgBase);
    pal.setColor(QPalette::WindowText, C::text);
    pal.setColor(QPalette::Base, C::bgBase);
    pal.setColor(QPalette::AlternateBase, C::bgAlt);
    pal.setColor(QPalette::Text, C::text);
    pal.setColor(QPalette::Button, C::bgAlt);
    pal.setColor(QPalette::ButtonText, C::text);
    pal.setColor(QPalette::Highlight, C::editRow);
    pal.setColor(QPalette::HighlightedText, C::text);
    w->setPalette(pal);
}

}
