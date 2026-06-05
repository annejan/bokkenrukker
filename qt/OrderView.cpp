#include "OrderView.h"
#include "SdlKeyMap.h"

#include <QPainter>
#include <QKeyEvent>
#include <QFontDatabase>
#include <QFontMetrics>

extern "C" {
#include "gcommon.h"
extern unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
extern int songlen[MAX_SONGS][MAX_CHN];
extern int espos[MAX_CHN];
extern int esend[MAX_CHN];
extern int esview;
extern int escolumn;
extern int eschn;
extern int esnum;
extern char songname[MAX_STR];
void orderlistcommands(void);
}

OrderView::OrderView(QWidget *parent) : QWidget(parent) {
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

static QString fmtOrderByte(unsigned char v) {
    if (v == LOOPSONG) return "RST";
    if (v >= TRANSUP) {
        int u = v - TRANSUP;
        return QString("+%1").arg(u, 1, 16).toUpper();
    }
    if (v >= TRANSDOWN) {
        int d = v - TRANSDOWN;
        return QString("-%1").arg(d, 1, 16).toUpper();
    }
    if (v >= REPEAT) {
        int r = v - REPEAT;
        return QString("R%1").arg(r, 1, 16).toUpper();
    }
    return QString("%1").arg(v, 2, 16, QLatin1Char('0')).toUpper();
}

void OrderView::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setFont(font());
    int x0 = 8, y0 = 8;

    p.setPen(QColor(180, 180, 220));
    p.drawText(QPoint(x0, y0 + rowHeight),
               QString("SONG %1 — %2")
                  .arg(esnum + 1)
                  .arg(QString::fromLocal8Bit(songname)));

    int gridY = y0 + rowHeight * 2 + 4;
    int colW = 6 * colWidth + 8;

    p.setPen(QColor(140, 160, 200));
    for (int c = 0; c < MAX_CHN; c++) {
        p.drawText(QPoint(x0 + 5 * colWidth + c * colW, gridY),
                   QString("Channel %1").arg(c + 1));
    }
    gridY += rowHeight;

    int maxLen = 0;
    for (int c = 0; c < MAX_CHN; c++)
        if (songlen[esnum][c] > maxLen) maxLen = songlen[esnum][c];
    maxLen += 2; // include RST + restart pos

    int visibleRows = (height() - gridY - 32) / rowHeight;
    if (visibleRows < 1) visibleRows = 1;
    int start = esview;
    if (start > maxLen - visibleRows) start = maxLen - visibleRows;
    if (start < 0) start = 0;

    for (int r = 0; r < visibleRows; r++) {
        int row = start + r;
        if (row >= maxLen) break;
        int y = gridY + r * rowHeight;
        p.setPen(QColor(120, 120, 110));
        p.drawText(QPoint(x0, y + rowHeight - 4),
                   QString("%1").arg(row, 3, 16, QLatin1Char('0')).toUpper());

        for (int c = 0; c < MAX_CHN; c++) {
            int xx = x0 + 5 * colWidth + c * colW;
            unsigned char v = songorder[esnum][c][row];
            bool isCursor = (row == espos[c] && c == eschn);
            QRect cell(xx - 2, y, 5 * colWidth, rowHeight);
            if (isCursor) p.fillRect(cell, QColor(40, 60, 80));

            QColor col(220, 220, 210);
            if (row >= songlen[esnum][c] + 2) col = QColor(60, 60, 60);
            else if (v == LOOPSONG) col = QColor(220, 150, 80);
            else if (v >= TRANSDOWN) col = QColor(150, 200, 230);
            else if (v >= REPEAT) col = QColor(200, 200, 100);
            p.setPen(col);
            p.drawText(QPoint(xx, y + rowHeight - 4), fmtOrderByte(v));
        }
    }

    p.setPen(QColor(140, 140, 160));
    p.drawText(QPoint(x0, height() - 8),
               "RET=goto patt   <>=subtune  SHIFT+R=repeat  +/-=transpose  SHIFT+1/2/3=swap channels");
}

bool OrderView::event(QEvent *e) {
    if (e->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(e);
        if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
            return false; // let MainWindow shortcut handle Tab
        }
    }
    return QWidget::event(e);
}

void OrderView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    orderlistcommands();
    clearGoatKeys();
    refresh();
    emit edited();
}
