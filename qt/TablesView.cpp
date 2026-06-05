#include "TablesView.h"
#include "SdlKeyMap.h"
#include "Theme.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QFontMetrics>

extern "C" {
#include "gcommon.h"
extern unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
extern unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
extern int etnum;
extern int etpos;
extern int etcolumn;
extern int etview[MAX_TABLES];
extern int etlock;
void tablecommands(void);
}

TablesView::TablesView(QWidget *parent) : QWidget(parent) {
    QFont mono = Theme::monoFont(11);
    setFont(mono);
    QFontMetrics fm(mono);
    rowHeight = fm.height() + 2;
    colWidth = fm.horizontalAdvance('0');
    Theme::applyDarkPalette(this);
    setFocusPolicy(Qt::StrongFocus);
}

static const char *tableName[4] = {"Wave", "Pulse", "Filter", "Speed"};
static const QColor tableTint[4] = {
    QColor(0xA1, 0xC1, 0x81),
    QColor(0x6F, 0xA6, 0xCE),
    QColor(0xD8, 0x9B, 0x8C),
    QColor(0xE0, 0xC1, 0x6E)
};

// Compact per-table legend from readme.txt sections 3.4.1 - 3.4.4.
// Lines must fit ~22 chars at point 9 font (chnW = 14 * colW @ pt11).
static const char *legend[4][6] = {
    {  // Wave (3.4.1)
        "L 00    no change",
        "L 01-0F delay 1-15f",
        "L 10-DF waveform",
        "L E0-EF inaudible",
        "L F0-FE exec cmd",
        "L FF    jump  R=pos",
    },
    {  // Pulse (3.4.2)
        "L 01-7F modulate",
        "        R=signed spd",
        "L 8X-FX set pw",
        "        R=low byte",
        "L FF    jump  R=pos",
        "",
    },
    {  // Filter (3.4.3)
        "L 00    cutoff R=val",
        "L 01-7F modulate",
        "        R=signed spd",
        "L 80-F0 set params",
        "        R=res|mask",
        "L FF    jump  R=pos",
    },
    {  // Speed (3.4.4)
        "Vibrato:",
        "  L=spd R=depth",
        "Portamento:",
        "  LR=16bit step",
        "Funktempo:",
        "  L=evn R=odd",
    },
};

static const char *legendR[4] = {
    "R 00-5F rel  80 hold  81-DF abs",
    nullptr,
    nullptr,
    "L bit $80: note-indep",
};

// Decode the current cell for the contextual help footer.
static QString decodeCell(int t, unsigned char L, unsigned char R) {
    QString line;
    auto hex = [](unsigned v, int w = 2) {
        return QString("%1").arg(v, w, 16, QLatin1Char('0')).toUpper();
    };
    if (t == 0) { // WAVE
        if (L == 0x00) line = "no change";
        else if (L >= 0x01 && L <= 0x0F) line = QString("delay %1 frames").arg(L);
        else if (L >= 0xE0 && L <= 0xEF) line = QString("inaudible $%1").arg(hex(L - 0xE0));
        else if (L == 0xFF) line = R == 0 ? "stop" : QString("jump to step $%1").arg(hex(R));
        else if (L >= 0xF0 && L <= 0xFE) line = QString("execute cmd %1XY param $%2")
                                                .arg(L - 0xF0, 1, 16).arg(hex(R));
        else {
            QStringList bits;
            if (L & 0x80) bits << "noise";
            if (L & 0x40) bits << "pulse";
            if (L & 0x20) bits << "saw";
            if (L & 0x10) bits << "tri";
            if (L & 0x08) bits << "test";
            if (L & 0x04) bits << "ring";
            if (L & 0x02) bits << "sync";
            if (L & 0x01) bits << "gate";
            line = QString("wave $%1 [%2]").arg(hex(L)).arg(bits.join('+'));
        }
        // Right side decode
        QString rs;
        if (R == 0x80) rs = "hold freq";
        else if (R >= 0x81) rs = QString("abs note %1").arg(R - 0x81);
        else if (R >= 0x60) rs = QString("neg rel %1").arg(R - 0x60);
        else rs = QString("rel +%1").arg(R);
        line += "    note: " + rs;
    } else if (t == 1) { // PULSE
        if (L == 0xFF) line = R == 0 ? "stop" : QString("jump to step $%1").arg(hex(R));
        else if (L >= 0x80) {
            int pw = ((L & 0x0F) << 8) | R;
            line = QString("set pulse width $%1").arg(hex(pw, 3));
        } else {
            int spd = (signed char)R;
            line = QString("modulate %1 ticks @ %2%3").arg(L)
                   .arg(spd >= 0 ? "+" : "").arg(spd);
        }
    } else if (t == 2) { // FILTER
        if (L == 0x00) line = QString("set cutoff $%1").arg(hex(R));
        else if (L == 0xFF) line = R == 0 ? "stop" : QString("jump to step $%1").arg(hex(R));
        else if (L >= 0x80) {
            const char *pass = "??";
            switch (L & 0xF0) {
                case 0x80: pass = "off";      break;
                case 0x90: pass = "lowpass";  break;
                case 0xA0: pass = "bandpass"; break;
                case 0xB0: pass = "low+band"; break;
                case 0xC0: pass = "highpass"; break;
                case 0xD0: pass = "low+high"; break;
                case 0xE0: pass = "high+bnd"; break;
                case 0xF0: pass = "all-pass"; break;
            }
            int res = (R >> 4) & 0xf;
            int mask = R & 0xf;
            line = QString("filter %1, res=$%2, ch-mask=%3")
                   .arg(pass).arg(res, 1, 16).arg(mask, 1, 16);
        } else {
            int spd = (signed char)R;
            line = QString("modulate %1 ticks @ %2%3").arg(L)
                   .arg(spd >= 0 ? "+" : "").arg(spd);
        }
    } else { // SPEED
        bool noteIndep = (L & 0x80) != 0;
        if (noteIndep) {
            line = QString("note-indep, divisor %1").arg(L & 0x7f);
        } else {
            line = QString("vib spd=%1 depth=%2  porta=%3  funk evn=%4 odd=%5")
                   .arg(L, 1, 16).arg(R, 1, 16)
                   .arg(((unsigned)L << 8) | R, 4, 16, QLatin1Char('0'))
                   .arg(L, 2, 16, QLatin1Char('0'))
                   .arg(R, 2, 16, QLatin1Char('0')).toUpper();
        }
    }
    return line;
}

void TablesView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setFont(font());
    int x0 = 16, y0 = 12;
    int colW = 14 * colWidth;

    p.setPen(Theme::C::textDim);
    p.drawText(QPoint(x0, y0 + rowHeight),
               QString("BACKQUOTE=switch   SHIFT+U=%1 scroll   active: %2")
                   .arg(etlock ? "lock" : "unlock")
                   .arg(tableName[etnum]));

    int headerY = y0 + rowHeight * 2 + 6;
    for (int t = 0; t < MAX_TABLES; t++) {
        int x = x0 + t * colW;
        bool active = (t == etnum);
        QRect titleRect(x - 4, headerY - rowHeight + 4, colW - 6, rowHeight);
        if (active) {
            p.fillRect(titleRect, Theme::C::editRow);
            p.setPen(QPen(Theme::C::highlight, 2));
            p.drawRect(titleRect.adjusted(0, 0, -1, -1));
        }
        p.setPen(active ? Theme::C::highlight : tableTint[t]);
        QFont hf = font();
        hf.setBold(active);
        p.setFont(hf);
        p.drawText(QPoint(x, headerY - 2),
                   QString("%1 table").arg(tableName[t]));
    }
    p.setFont(font());

    // Compact per-table legend block under headers
    QFont small = font(); small.setPointSize(9);
    p.setFont(small);
    int legendY = headerY + rowHeight + 4;
    int legendLines = 6;
    int legendH = legendLines * (small.pointSize() + 4);
    for (int t = 0; t < MAX_TABLES; t++) {
        int x = x0 + t * colW;
        for (int i = 0; i < legendLines; i++) {
            p.setPen(t == etnum ? Theme::C::text : Theme::C::textDim);
            p.drawText(QPoint(x, legendY + i * (small.pointSize() + 4)),
                       legend[t][i]);
        }
        if (legendR[t]) {
            p.setPen(t == etnum ? Theme::C::text : Theme::C::textDim);
            p.drawText(QPoint(x, legendY + legendLines * (small.pointSize() + 4)),
                       legendR[t]);
        }
    }
    p.setFont(font());

    int gridY = legendY + legendH + 14;
    int rows = (height() - gridY - 56) / rowHeight;
    if (rows < 1) rows = 1;
    if (rows > MAX_TABLELEN) rows = MAX_TABLELEN;

    for (int t = 0; t < MAX_TABLES; t++) {
        int x = x0 + t * colW;
        int view = etlock ? etview[etnum] : etview[t];
        for (int r = 0; r < rows; r++) {
            int idx = view + r;
            if (idx >= MAX_TABLELEN) break;
            int y = gridY + r * rowHeight;
            if (r % 4 == 0)
                p.fillRect(QRect(x - 4, y, colW - 6, rowHeight), Theme::C::beat);
            bool cursor = (t == etnum && idx == etpos);
            if (cursor) p.fillRect(QRect(x - 4, y, colW - 6, rowHeight), Theme::C::editRow);
            p.setPen(Theme::C::textDim);
            p.drawText(QPoint(x, y + rowHeight - 4),
                       QString("%1").arg(idx + 1, 2, 16, QLatin1Char('0')).toUpper());
            unsigned char L = ltable[t][idx];
            unsigned char R = rtable[t][idx];
            p.setPen(L ? Theme::C::cmdDigit : Theme::C::textDim);
            p.drawText(QPoint(x + 4 * colWidth, y + rowHeight - 4),
                       QString("%1").arg(L, 2, 16, QLatin1Char('0')).toUpper());
            p.setPen(R ? Theme::C::cmdParam : Theme::C::textDim);
            p.drawText(QPoint(x + 7 * colWidth, y + rowHeight - 4),
                       QString("%1").arg(R, 2, 16, QLatin1Char('0')).toUpper());
        }
    }

    // Cursor decode footer — what does the active cell mean?
    unsigned char Lc = ltable[etnum][etpos];
    unsigned char Rc = rtable[etnum][etpos];
    int footerY = height() - 40;
    QRect footRect(8, footerY, width() - 16, 30);
    p.fillRect(footRect, Theme::C::bgAlt);
    p.setPen(Theme::C::sep);
    p.drawRect(footRect);
    p.setPen(Theme::C::highlight);
    QFont bf = font(); bf.setBold(true);
    p.setFont(bf);
    QString head = QString("%1 step $%2   L=$%3  R=$%4")
                   .arg(tableName[etnum])
                   .arg(etpos + 1, 2, 16, QLatin1Char('0'))
                   .arg(Lc, 2, 16, QLatin1Char('0'))
                   .arg(Rc, 2, 16, QLatin1Char('0')).toUpper();
    p.drawText(QPoint(16, footerY + 13), head);
    p.setFont(font());
    p.setPen(Theme::C::text);
    p.drawText(QPoint(16, footerY + 26), "→ " + decodeCell(etnum, Lc, Rc));

    p.setPen(Theme::C::textDim);
    p.drawText(QPoint(x0, height() - 8),
               "RET=back to instr   SHIFT+N=negate   SHIFT+O=optimize   SHIFT+L=convert limit→time");
}

void TablesView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    tablecommands();
    clearGoatKeys();
    refresh();
    emit edited();
}
