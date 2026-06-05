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
    QColor(0xA1, 0xC1, 0x81),  // wave green
    QColor(0x6F, 0xA6, 0xCE),  // pulse blue
    QColor(0xD8, 0x9B, 0x8C),  // filter rust
    QColor(0xE0, 0xC1, 0x6E)   // speed yellow
};

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
        // Group-box title rect
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

    int gridY = headerY + rowHeight + 4;
    int rows = (height() - gridY - 32) / rowHeight;
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

    p.setPen(Theme::C::textDim);
    p.drawText(QPoint(x0, height() - 10),
               "RET=back to instr   SHIFT+N=negate   SHIFT+O=optimize   SHIFT+L=convert limit→time");
}

void TablesView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    tablecommands();
    clearGoatKeys();
    refresh();
    emit edited();
}
