#include "TablesView.h"
#include "SdlKeyMap.h"

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
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(11);
    setFont(mono);
    QFontMetrics fm(mono);
    rowHeight = fm.height();
    colWidth = fm.horizontalAdvance('0');
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(20, 24, 30));
    pal.setColor(QPalette::WindowText, QColor(220, 220, 200));
    setPalette(pal);
    setFocusPolicy(Qt::StrongFocus);
}

static const char *tableName[4] = {"Wave", "Pulse", "Filter", "Speed"};

void TablesView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setFont(font());
    int x0 = 16, y0 = 16;
    int colW = 14 * colWidth;

    p.setPen(QColor(160, 200, 240));
    p.drawText(QPoint(x0, y0 + rowHeight),
               QString("BACKQUOTE=switch table  SHIFT+U=lock/unlock scroll  Active table: %1")
                   .arg(tableName[etnum]));

    int headerY = y0 + rowHeight * 2 + 4;
    for (int t = 0; t < MAX_TABLES; t++) {
        int x = x0 + t * colW;
        QColor c = (t == etnum) ? QColor(220, 220, 80) : QColor(150, 150, 170);
        p.setPen(c);
        p.drawText(QPoint(x, headerY), QString("%1 table").arg(tableName[t]));
    }

    int gridY = headerY + rowHeight;
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
            bool cursor = (t == etnum && idx == etpos);
            if (cursor) p.fillRect(QRect(x - 2, y, colW - 4, rowHeight), QColor(40, 60, 80));
            p.setPen(QColor(120, 120, 110));
            p.drawText(QPoint(x, y + rowHeight - 4),
                       QString("%1").arg(idx + 1, 2, 16, QLatin1Char('0')).toUpper());
            p.setPen(QColor(220, 220, 230));
            p.drawText(QPoint(x + 4 * colWidth, y + rowHeight - 4),
                       QString("%1 %2")
                           .arg(ltable[t][idx], 2, 16, QLatin1Char('0'))
                           .arg(rtable[t][idx], 2, 16, QLatin1Char('0'))
                           .toUpper());
        }
    }

    p.setPen(QColor(140, 140, 160));
    p.drawText(QPoint(x0, height() - 8),
               "RET=back to instr  SHIFT+N=negate  SHIFT+O=optimize  SHIFT+L=convert limit→time");
}

void TablesView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    tablecommands();
    clearGoatKeys();
    refresh();
    emit edited();
}
