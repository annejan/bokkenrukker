#include "OrderView.h"
#include "SdlKeyMap.h"
#include "Theme.h"

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
    QFont mono = Theme::monoFont(11);
    setFont(mono);
    QFontMetrics fm(mono);
    rowHeight = fm.height() + 3;
    colWidth = fm.horizontalAdvance('0');
    Theme::applyDarkPalette(this);
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
    int x0 = 12, y0 = 10;

    p.setPen(Theme::C::highlight);
    QFont bf = font();
    bf.setBold(true);
    p.setFont(bf);
    p.drawText(QPoint(x0, y0 + rowHeight),
               QString("SONG %1 — %2")
                  .arg(esnum + 1)
                  .arg(QString::fromLocal8Bit(songname)));
    p.setFont(font());

    int gridY = y0 + rowHeight * 2 + 4;
    // Header "Channel N" is 9 chars; pattern values are 3 chars. Use 11-char column.
    int colW = 11 * colWidth + 8;

    for (int c = 0; c < MAX_CHN; c++) {
        bool active = (c == eschn);
        p.setPen(active ? Theme::C::highlight : Theme::C::textDim);
        QFont hf = font();
        hf.setBold(active);
        p.setFont(hf);
        p.drawText(QPoint(x0 + 5 * colWidth + c * colW, gridY),
                   QString("Channel %1").arg(c + 1));
    }
    p.setFont(font());
    gridY += rowHeight;
    p.setPen(Theme::C::sep);
    p.drawLine(x0, gridY - 4, x0 + 5 * colWidth + MAX_CHN * colW, gridY - 4);

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
        if (row % 4 == 0)
            p.fillRect(QRect(x0 - 4, y, 5 * colWidth + MAX_CHN * colW + 8, rowHeight),
                       Theme::C::beat);
        p.setPen(Theme::C::textDim);
        p.drawText(QPoint(x0, y + rowHeight - 5),
                   QString("%1").arg(row, 3, 16, QLatin1Char('0')).toUpper());

        for (int c = 0; c < MAX_CHN; c++) {
            int xx = x0 + 5 * colWidth + c * colW;
            unsigned char v = songorder[esnum][c][row];
            bool isCursor = (row == espos[c] && c == eschn);
            QRect cell(xx - 2, y, 5 * colWidth, rowHeight);
            if (isCursor) p.fillRect(cell, Theme::C::editRow);

            QColor col = Theme::C::text;
            if (row >= songlen[esnum][c] + 2) col = Theme::C::textDim;
            else if (v == LOOPSONG) col = Theme::C::rst;
            else if (v >= TRANSDOWN) col = Theme::C::transpose;
            else if (v >= REPEAT) col = Theme::C::repeatCol;
            p.setPen(col);
            p.drawText(QPoint(xx, y + rowHeight - 5), fmtOrderByte(v));
        }
    }

    p.setPen(Theme::C::textDim);
    p.drawText(QPoint(x0, height() - 10),
               "RET=goto patt   <>=subtune   SHIFT+R=repeat   +/-=transpose   SHIFT+1/2/3=swap channels");
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
