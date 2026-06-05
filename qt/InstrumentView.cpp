#include "InstrumentView.h"
#include "SdlKeyMap.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QFontMetrics>
#include <cstring>

extern "C" {
#include "gcommon.h"
extern INSTR instr[MAX_INSTR];
extern int einum;
extern int eipos;
extern int eicolumn;
extern int epoctave;
void instrumentcommands(void);
}

InstrumentView::InstrumentView(QWidget *parent) : QWidget(parent) {
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(11);
    setFont(mono);
    QFontMetrics fm(mono);
    rowHeight = fm.height() + 2;
    colWidth = fm.horizontalAdvance('0');
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(20, 24, 30));
    pal.setColor(QPalette::WindowText, QColor(220, 220, 200));
    setPalette(pal);
    setFocusPolicy(Qt::StrongFocus);
}

static const char *paramName[] = {
    "Attack/Decay",
    "Sustain/Release",
    "Wavetable Pos",
    "Pulsetable Pos",
    "Filtertable Pos",
    "Vibrato Param",
    "Vibrato Delay",
    "HR/Gate Timer",
    "1stFrame Wave",
};

void InstrumentView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setFont(font());
    int x = 16, y = 16;

    INSTR &ins = instr[einum];
    char nameBuf[MAX_INSTRNAMELEN + 1];
    std::memcpy(nameBuf, ins.name, MAX_INSTRNAMELEN);
    nameBuf[MAX_INSTRNAMELEN] = 0;

    p.setPen(QColor(180, 220, 255));
    p.drawText(QPoint(x, y + rowHeight),
               QString("INSTRUMENT %1: %2")
                  .arg(einum, 2, 16, QLatin1Char('0')).toUpper()
                  .arg(QString::fromLocal8Bit(nameBuf)));
    p.setPen(QColor(160, 160, 180));
    p.drawText(QPoint(x, y + rowHeight * 2),
               QString("Octave: %1   (SHIFT+N edit name, +/- inc/dec instr)").arg(epoctave));

    int gridY = y + rowHeight * 4;
    unsigned char params[9] = {
        ins.ad, ins.sr, ins.ptr[WTBL], ins.ptr[PTBL], ins.ptr[FTBL],
        ins.ptr[STBL], ins.vibdelay, ins.gatetimer, ins.firstwave
    };
    for (int i = 0; i < 9; i++) {
        int yy = gridY + i * rowHeight;
        bool cursor = (i == eipos);
        if (cursor) p.fillRect(QRect(x - 4, yy, 380, rowHeight), QColor(40, 60, 80));
        p.setPen(QColor(180, 180, 180));
        p.drawText(QPoint(x, yy + rowHeight - 4), paramName[i]);
        p.setPen(QColor(220, 220, 230));
        p.drawText(QPoint(x + 24 * colWidth, yy + rowHeight - 4),
                   QString("%1").arg(params[i], 2, 16, QLatin1Char('0')).toUpper());
    }

    p.setPen(QColor(140, 140, 160));
    p.drawText(QPoint(x, height() - 8),
               "RET=goto table  SPACE=test note  SHIFT+N=name  SHIFT+X/C/V=cut/copy/paste");
}

void InstrumentView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    instrumentcommands();
    clearGoatKeys();
    refresh();
    emit edited();
}
