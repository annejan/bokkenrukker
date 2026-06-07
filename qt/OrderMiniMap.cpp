#include "OrderMiniMap.h"
#include "Theme.h"

#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>
#include <QEvent>

extern "C" {
#include "gcommon.h"
#include "gplay.h"
extern unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
extern int songlen[MAX_SONGS][MAX_CHN];
extern int esnum;
extern int espos[MAX_CHN];
extern int eschn;
extern int lastsonginit;
extern int stereo_mode;
extern CHN chn[MAX_CHN];
int isplaying(void);
}

static int activeChannels() { return stereo_mode ? MAX_CHN : 3; }

OrderMiniMap::OrderMiniMap(QWidget *parent) : QWidget(parent) {
    // No hard minimum — the previous 120 px floor stopped the user from
    // collapsing the Order map dock to zero (and prevented the pattern
    // editor from claiming the freed space when 'View > Order map' was
    // toggled). Keep a soft hint via sizeHint() instead.
    setMaximumWidth(160);
    setMouseTracking(true);
    Theme::applyDarkPalette(this);
}

void OrderMiniMap::paintEvent(QPaintEvent *) {
    QPainter p(this);
    int W = width(), H = height();
    int nc = activeChannels();
    int maxLen = 0;
    for (int c = 0; c < nc; c++)
        if (songlen[esnum][c] > maxLen) maxLen = songlen[esnum][c];
    if (maxLen == 0) maxLen = 1;
    int rowH = qMax(2, H / qMax(maxLen, 1));
    int colW = (W - 8) / nc;

    p.setPen(Theme::C::textDim);
    QFont f = font(); f.setPointSize(8); p.setFont(f);
    p.drawText(QRect(0, 0, W, 14), Qt::AlignCenter,
               QString("ORDER %1").arg(esnum + 1));

    bool playing = isplaying() != 0;
    int startY = 18;
    for (int c = 0; c < MAX_CHN; c++) {
        int x = 4 + c * colW;
        // chn[c].songptr is the *next* slot to fetch in song-play modes; the
        // one currently playing is the slot before. In PLAY_PATTERN mode
        // initsong wipes songptr to 0, so use the user-selected espos[c]
        // instead — that's the orderlist row the user clicked to play.
        int playRow = -1;
        if (playing) {
            if (lastsonginit == PLAY_PATTERN) playRow = espos[c];
            else                              playRow = (int)chn[c].songptr - 1;
            if (playRow < 0) playRow = 0;
        }
        for (int r = 0; r < songlen[esnum][c]; r++) {
            unsigned char v = songorder[esnum][c][r];
            int y = startY + r * rowH;
            QColor cell = Theme::C::bgAlt;
            if (v == LOOPSONG)       cell = Theme::C::rst;
            else if (v >= TRANSDOWN) cell = Theme::C::transpose;
            else if (v >= REPEAT)    cell = Theme::C::repeatCol;
            else {
                int t = qMin(255, 40 + v * 4);
                cell = QColor(t / 3, t / 2, t);
            }
            p.fillRect(QRect(x, y, colW - 2, rowH - 1), cell);
            // Playback row — red fill underlay
            if (playing && r == playRow) {
                p.fillRect(QRect(x, y, colW - 2, rowH - 1), Theme::C::playRow);
                p.setPen(QPen(Theme::C::vuRed, 1));
                p.drawRect(QRect(x, y, colW - 2, rowH - 1));
            }
            // Edit cursor — yellow outline
            if (r == espos[c] && c == eschn) {
                p.setPen(QPen(Theme::C::highlight, 2));
                p.drawRect(QRect(x, y, colW - 2, rowH - 1));
            }
        }
    }
}

extern "C" {
extern int epchn;
extern int epnum[MAX_CHN];
extern int eppos;
extern int eseditpos;
}

void OrderMiniMap::mousePressEvent(QMouseEvent *e) {
    int W = width();
    int nc = activeChannels();
    int colW = (W - 8) / nc;
    int x = e->pos().x() - 4;
    if (x < 0) return;
    int c = x / colW;
    if (c < 0 || c >= nc) return;
    int maxLen = 0;
    for (int cc = 0; cc < nc; cc++)
        if (songlen[esnum][cc] > maxLen) maxLen = songlen[esnum][cc];
    int rowH = qMax(2, height() / qMax(maxLen, 1));
    int r = (e->pos().y() - 18) / rowH;
    if (r < 0) return;

    // The dock header toggle drives the default action; ctrl inverts it so
    // both modes stay reachable from the keyboard. selectAll_ = true keeps
    // the legacy 'plain click = all channels' behaviour.
    bool ctrlMod = (e->modifiers() & Qt::ControlModifier) != 0;
    bool moveAll = selectAll_ ^ ctrlMod;

    auto apply = [&](int channel) {
        if (r >= songlen[esnum][channel]) return;
        espos[channel] = r;
        unsigned char v = songorder[esnum][channel][r];
        if (v < REPEAT) { // skip RST/transpose/repeat
            epnum[channel] = v;
            eppos = 0;
        }
    };

    if (moveAll) {
        for (int cc = 0; cc < nc; cc++) apply(cc);
    } else {
        apply(c);
    }
    eschn = c;
    epchn = c;
    eseditpos = r;
    update();
    emit positionChanged();
}

void OrderMiniMap::mouseMoveEvent(QMouseEvent *e) {
    int W = width();
    int nc = activeChannels();
    int colW = (W - 8) / nc;
    int x = e->pos().x() - 4;
    int c = (colW > 0) ? x / colW : -1;
    if (x < 0 || c < 0 || c >= nc) { QToolTip::hideText(); return; }

    int maxLen = 0;
    for (int cc = 0; cc < nc; cc++)
        if (songlen[esnum][cc] > maxLen) maxLen = songlen[esnum][cc];
    int rowH = qMax(2, height() / qMax(maxLen, 1));
    int r = (e->pos().y() - 18) / rowH;
    if (r < 0 || r >= songlen[esnum][c]) { QToolTip::hideText(); return; }

    unsigned char v = songorder[esnum][c][r];
    QString tip;
    if (v == LOOPSONG)       tip = QString("CH %1  row %2  RST (loop)").arg(c + 1).arg(r);
    else if (v >= TRANSDOWN) tip = QString("CH %1  row %2  Transpose -%3")
                                   .arg(c + 1).arg(r).arg(0xff - v, 2, 16, QLatin1Char('0')).toUpper();
    else if (v >= TRANSUP)   tip = QString("CH %1  row %2  Transpose +%3")
                                   .arg(c + 1).arg(r).arg(v - TRANSUP, 2, 16, QLatin1Char('0')).toUpper();
    else if (v >= REPEAT)    tip = QString("CH %1  row %2  Repeat x%3")
                                   .arg(c + 1).arg(r).arg(v - REPEAT + 1);
    else                     tip = QString("CH %1  row %2  pattern $%3 (%4)")
                                   .arg(c + 1).arg(r)
                                   .arg(v, 2, 16, QLatin1Char('0')).toUpper().arg(v);
    QToolTip::showText(e->globalPosition().toPoint(), tip, this);
}

void OrderMiniMap::leaveEvent(QEvent *) {
    QToolTip::hideText();
}
