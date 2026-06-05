#include "OrderMiniMap.h"
#include "Theme.h"

#include <QPainter>
#include <QMouseEvent>

extern "C" {
#include "gcommon.h"
extern unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
extern int songlen[MAX_SONGS][MAX_CHN];
extern int esnum;
extern int espos[MAX_CHN];
extern int eschn;
}

OrderMiniMap::OrderMiniMap(QWidget *parent) : QWidget(parent) {
    setMinimumWidth(120);
    setMaximumWidth(160);
    Theme::applyDarkPalette(this);
}

void OrderMiniMap::paintEvent(QPaintEvent *) {
    QPainter p(this);
    int W = width(), H = height();
    int maxLen = 0;
    for (int c = 0; c < MAX_CHN; c++)
        if (songlen[esnum][c] > maxLen) maxLen = songlen[esnum][c];
    if (maxLen == 0) maxLen = 1;
    int rowH = qMax(2, H / qMax(maxLen, 1));
    int colW = (W - 8) / MAX_CHN;

    p.setPen(Theme::C::textDim);
    QFont f = font(); f.setPointSize(8); p.setFont(f);
    p.drawText(QRect(0, 0, W, 14), Qt::AlignCenter,
               QString("ORDER %1").arg(esnum + 1));

    int startY = 18;
    for (int c = 0; c < MAX_CHN; c++) {
        int x = 4 + c * colW;
        for (int r = 0; r < songlen[esnum][c]; r++) {
            unsigned char v = songorder[esnum][c][r];
            int y = startY + r * rowH;
            QColor cell = Theme::C::bgAlt;
            if (v == LOOPSONG)       cell = Theme::C::rst;
            else if (v >= TRANSDOWN) cell = Theme::C::transpose;
            else if (v >= REPEAT)    cell = Theme::C::repeatCol;
            else {
                // Pattern-number cell — brightness encodes value
                int t = qMin(255, 40 + v * 4);
                cell = QColor(t / 3, t / 2, t);
            }
            p.fillRect(QRect(x, y, colW - 2, rowH - 1), cell);
            if (r == espos[c] && c == eschn) {
                p.setPen(QPen(Theme::C::highlight, 2));
                p.drawRect(QRect(x, y, colW - 2, rowH - 1));
            }
        }
    }
}

void OrderMiniMap::mousePressEvent(QMouseEvent *e) {
    int W = width();
    int colW = (W - 8) / MAX_CHN;
    int x = e->pos().x() - 4;
    if (x < 0) return;
    int c = x / colW;
    if (c < 0 || c >= MAX_CHN) return;
    int maxLen = 0;
    for (int cc = 0; cc < MAX_CHN; cc++)
        if (songlen[esnum][cc] > maxLen) maxLen = songlen[esnum][cc];
    int rowH = qMax(2, height() / qMax(maxLen, 1));
    int r = (e->pos().y() - 18) / rowH;
    if (r < 0 || r >= songlen[esnum][c]) return;
    eschn = c;
    espos[c] = r;
    update();
    emit positionChanged();
}
