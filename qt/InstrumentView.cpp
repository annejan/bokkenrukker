#include "InstrumentView.h"
#include "SdlKeyMap.h"
#include "Theme.h"

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
    QFont mono = Theme::monoFont(11);
    setFont(mono);
    QFontMetrics fm(mono);
    rowHeight = fm.height() + 3;
    colWidth = fm.horizontalAdvance('0');
    Theme::applyDarkPalette(this);
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

    p.setPen(Theme::C::highlight);
    QFont bf = font();
    bf.setBold(true);
    p.setFont(bf);
    p.drawText(QPoint(x, y + rowHeight),
               QString("INSTRUMENT %1: %2")
                  .arg(einum, 2, 16, QLatin1Char('0')).toUpper()
                  .arg(QString::fromLocal8Bit(nameBuf)));
    p.setFont(font());
    p.setPen(Theme::C::textDim);
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
        if (cursor) p.fillRect(QRect(x - 4, yy, 380, rowHeight), Theme::C::editRow);
        p.setPen(Theme::C::textDim);
        p.drawText(QPoint(x, yy + rowHeight - 5), paramName[i]);
        p.setPen(Theme::C::text);
        p.drawText(QPoint(x + 24 * colWidth, yy + rowHeight - 5),
                   QString("%1").arg(params[i], 2, 16, QLatin1Char('0')).toUpper());
    }

    // ADSR envelope visualization
    int envX = x + 380;
    int envY = gridY;
    int envW = 280;
    int envH = 140;
    QRect envRect(envX, envY, envW, envH);
    p.fillRect(envRect, Theme::C::bgAlt);
    p.setPen(Theme::C::sep);
    p.drawRect(envRect);
    int A = (ins.ad >> 4) & 0xf;
    int D = ins.ad & 0xf;
    int S = (ins.sr >> 4) & 0xf;
    int R = ins.sr & 0xf;
    int aw = 20 + A * 12;
    int dw = 20 + D * 10;
    int rw = 20 + R * 10;
    int sustY = envY + envH - 8 - (S * (envH - 16) / 15);
    int baseY = envY + envH - 8;
    int peakY = envY + 8;
    QPoint p0(envX + 8, baseY);
    QPoint p1(p0.x() + aw, peakY);
    QPoint p2(p1.x() + dw, sustY);
    QPoint p3(p2.x() + 60, sustY); // sustain held
    QPoint p4(p3.x() + rw, baseY);
    if (p4.x() > envX + envW - 8) p4.setX(envX + envW - 8);
    p.setPen(QPen(Theme::C::highlight, 2));
    p.drawLine(p0, p1);
    p.drawLine(p1, p2);
    p.drawLine(p2, p3);
    p.drawLine(p3, p4);
    p.setPen(Theme::C::textDim);
    p.drawText(QPoint(envX + 6, envY + 14), QString("ADSR  A%1 D%2 S%3 R%4")
        .arg(A,1,16).arg(D,1,16).arg(S,1,16).arg(R,1,16).toUpper());

    p.setPen(Theme::C::textDim);
    p.drawText(QPoint(x, height() - 10),
               "RET=goto table   SPACE=test note   SHIFT+N=name   SHIFT+X/C/V=cut/copy/paste");
}

void InstrumentView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    instrumentcommands();
    clearGoatKeys();
    refresh();
    emit edited();
}
