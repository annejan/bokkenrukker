#include "PatternView.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QScrollBar>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QInputDialog>
#include "SdlKeyMap.h"
#include "Theme.h"

extern "C" {
#include "gcommon.h"
#include "gplay.h"
#include "gsid.h"
extern unsigned char pattern[MAX_PATT][MAX_PATTROWS*4+4];
extern int pattlen[MAX_PATT];
extern int epnum[MAX_CHN];
extern int eppos;
extern int epview;
extern int epcolumn;
extern int epchn;
extern int recordmode;
extern int followplay;
extern int epmarkchn;
extern int epmarkstart;
extern int epmarkend;
extern CHN chn[MAX_CHN];
int isplaying(void);
void patterncommands(void);
void mutechannel(int chnnum);
}

static const char *notename[] = {
    "C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0",
    "C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1",
    "C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2",
    "C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3",
    "C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4",
    "C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5",
    "C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6",
    "C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","...","---","+++"
};

PatternView::PatternView(QWidget *parent) : QAbstractScrollArea(parent) {
    QFont mono = Theme::monoFont(11);
    setFont(mono);
    QFontMetrics fm(mono);
    rowHeight = fm.height() + 3;
    colWidth = fm.horizontalAdvance('0');
    rowNumW_ = 5 * colWidth;
    chnW_ = 12 * colWidth;
    vuStripH_ = 14;          // VU bar + small padding
    scopeStripH_ = 32;
    headerStripH_ = rowHeight + 4;
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Theme::applyDarkPalette(viewport());
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setFocusPolicy(Qt::StrongFocus);
    for (auto &row : scope_) row.fill(0);
    updateScrollRange();
}

int PatternView::visibleRows() const {
    return (viewport()->height() - gridTopOffset()) / rowHeight;
}

int PatternView::gridTopOffset() const {
    return vuStripH_ + scopeStripH_ + headerStripH_;
}

void PatternView::updateScrollRange() {
    int maxRows = 0;
    for (int c = 0; c < MAX_CHN; c++)
        if (pattlen[epnum[c]] > maxRows) maxRows = pattlen[epnum[c]];
    verticalScrollBar()->setRange(0, std::max(0, maxRows - visibleRows()));
    verticalScrollBar()->setPageStep(visibleRows());
}

void PatternView::resizeEvent(QResizeEvent *) {
    updateScrollRange();
}

void PatternView::refresh() {
    updateScrollRange();
    int rowOffset = verticalScrollBar()->value();
    int rows = visibleRows();
    bool playing = isplaying() != 0;
    // Centered follow-play scroll.
    if (followplay && playing) {
        int prow = chn[epchn].pattptr / 4;
        int center = std::max(0, prow - rows / 2);
        verticalScrollBar()->setValue(center);
    } else if (rows > 0) {
        if (eppos < rowOffset) verticalScrollBar()->setValue(eppos);
        else if (eppos >= rowOffset + rows)
            verticalScrollBar()->setValue(eppos - rows + 1);
    }
    viewport()->update();
}

void PatternView::tickScope() {
    unsigned char levels[3] = {0, 0, 0};
    sid_getlevels(levels);
    for (int c = 0; c < 3; c++) scope_[c][scopeHead_] = levels[c];
    scopeHead_ = (scopeHead_ + 1) % kScopeLen;
    viewport()->update();
}

bool PatternView::event(QEvent *e) {
    return QAbstractScrollArea::event(e);
}

void PatternView::keyPressEvent(QKeyEvent *e) {
    setGoatKeys(e);
    patterncommands();
    clearGoatKeys();
    refresh();
    emit patternEdited();
}

int PatternView::channelAtX(int x) const {
    int rx = x - rowNumW_;
    if (rx < 0) return -1;
    int c = rx / chnW_;
    if (c < 0 || c >= MAX_CHN) return -1;
    return c;
}

void PatternView::mousePressEvent(QMouseEvent *e) {
    const int y = e->pos().y();

    // Channel header strip — click on pattern# / mute toggle
    int headerY = vuStripH_ + scopeStripH_;
    if (y >= headerY && y < headerY + headerStripH_) {
        int c = channelAtX(e->pos().x());
        if (c >= 0) {
            int xInChan = e->pos().x() - (rowNumW_ + c * chnW_);
            int muteX = chnW_ - 24;
            if (xInChan >= muteX) {
                mutechannel(c);
                refresh();
                emit patternEdited();
                return;
            }
            // Click pattern# field — let user pick a new pattern
            if (xInChan >= 4 * colWidth && xInChan < 8 * colWidth) {
                bool ok = false;
                int n = QInputDialog::getInt(this, "Pattern number",
                    QString("Channel %1 pattern (hex):").arg(c + 1),
                    epnum[c], 0, MAX_PATT - 1, 1, &ok);
                if (ok) {
                    epnum[c] = n;
                    refresh();
                    emit patternEdited();
                }
                return;
            }
            // Plain click on header — switch active channel
            epchn = c;
            epcolumn = 0;
            refresh();
            emit patternEdited();
            return;
        }
    }

    // Grid area
    if (y >= gridTopOffset()) {
        int row = verticalScrollBar()->value()
                  + (y - gridTopOffset()) / rowHeight;
        if (row < 0) row = 0;
        if (row >= MAX_PATTROWS) row = MAX_PATTROWS - 1;
        eppos = row;
        int c = channelAtX(e->pos().x());
        if (c >= 0) {
            epchn = c;
            epcolumn = 0;
        }
        refresh();
        emit patternEdited();
    }
}

void PatternView::paintEvent(QPaintEvent *) {
    QPainter p(viewport());
    p.setFont(font());

    const int rowOffset = verticalScrollBar()->value();
    const int W = viewport()->width();

    // ---- VU bars strip --------------------------------------------------
    const int vuPad = 4;
    const int vuH = vuStripH_ - vuPad * 2;
    unsigned char levels[3] = {0, 0, 0};
    sid_getlevels(levels);
    for (int c = 0; c < MAX_CHN; c++) {
        int x = rowNumW_ + c * chnW_ + 2;
        int w = chnW_ - 6;
        QRect frame(x, vuPad, w, vuH);
        p.fillRect(frame, Theme::C::vuBg);
        p.setPen(Theme::C::sep);
        p.drawRect(frame);
        if (chn[c].mute) {
            p.setPen(QColor(120, 60, 60));
            p.drawText(QPoint(x + 4, vuPad + vuH - 2), "MUTE");
            continue;
        }
        int filled = (int)((double)levels[c] / 255.0 * (w - 2));
        if (filled > 0) {
            int seg1 = (int)((w - 2) * 0.6);
            int seg2 = (int)((w - 2) * 0.9);
            int rem = filled;
            int xx = x + 1;
            int g = qMin(rem, seg1);
            if (g > 0) { p.fillRect(QRect(xx, vuPad + 1, g, vuH - 2), Theme::C::vuGreen); rem -= g; xx += g; }
            int y = qMin(rem, seg2 - seg1);
            if (y > 0) { p.fillRect(QRect(xx, vuPad + 1, y, vuH - 2), Theme::C::vuAmber); rem -= y; xx += y; }
            if (rem > 0) p.fillRect(QRect(xx, vuPad + 1, rem, vuH - 2), Theme::C::vuRed);
        }
    }

    // ---- Mini scope strip -----------------------------------------------
    int scopeY = vuStripH_;
    for (int c = 0; c < MAX_CHN; c++) {
        int x = rowNumW_ + c * chnW_ + 2;
        int w = chnW_ - 6;
        QRect frame(x, scopeY + 2, w, scopeStripH_ - 4);
        p.fillRect(frame, Theme::C::vuBg);
        p.setPen(Theme::C::sep);
        p.drawRect(frame);
        QPainterPath path;
        int n = kScopeLen;
        bool first = true;
        for (int i = 0; i < n; i++) {
            int idx = (scopeHead_ + i) % n;
            float v = (float)scope_[c][idx] / 255.0f;
            float fx = frame.x() + 1 + (frame.width() - 2) * (float)i / (n - 1);
            float fy = frame.bottom() - 1 - v * (frame.height() - 2);
            if (first) { path.moveTo(fx, fy); first = false; }
            else        path.lineTo(fx, fy);
        }
        QColor stroke = chn[c].mute ? Theme::C::textDim : Theme::C::vuGreen;
        p.setPen(QPen(stroke, 1.5));
        p.setRenderHint(QPainter::Antialiasing, true);
        p.drawPath(path);
        p.setRenderHint(QPainter::Antialiasing, false);
    }

    // ---- Channel header strip -------------------------------------------
    int headerY = vuStripH_ + scopeStripH_;
    for (int c = 0; c < MAX_CHN; c++) {
        int x = rowNumW_ + c * chnW_ + 2;
        bool active = (c == epchn);
        QRect headRect(x - 2, headerY + 2, chnW_ - 4, headerStripH_ - 4);
        if (active) p.fillRect(headRect, Theme::C::bgAlt);

        p.setPen(active ? Theme::C::highlight : Theme::C::textDim);
        QFont hf = font();
        hf.setBold(active);
        p.setFont(hf);
        p.drawText(QPoint(x + 4, headerY + headerStripH_ - 6),
                   QString("Ch%1").arg(c + 1));
        p.setFont(font());

        // Pattern number (clickable)
        p.setPen(Theme::C::instr);
        p.drawText(QPoint(x + 4 * colWidth, headerY + headerStripH_ - 6),
                   QString("P%1").arg(epnum[c], 2, 16, QLatin1Char('0')).toUpper());
        // Length
        p.setPen(Theme::C::textDim);
        p.drawText(QPoint(x + 7 * colWidth + 4, headerY + headerStripH_ - 6),
                   QString("L%1").arg(pattlen[epnum[c]], 2, 16, QLatin1Char('0')).toUpper());
        // Mute toggle area
        int muteX = x + chnW_ - 28;
        QRect muteRect(muteX, headerY + 4, 22, headerStripH_ - 8);
        p.setPen(chn[c].mute ? Theme::C::vuRed : Theme::C::vuGreen);
        p.drawRect(muteRect);
        p.drawText(muteRect, Qt::AlignCenter, chn[c].mute ? "M" : "·");
    }
    p.setPen(Theme::C::sep);
    p.drawLine(0, headerY + headerStripH_, W, headerY + headerStripH_);

    // ---- Pattern grid ---------------------------------------------------
    const int topOffset = gridTopOffset();
    const int rows = (viewport()->height() - topOffset) / rowHeight;
    const bool playing = isplaying() != 0;

    for (int r = 0; r < rows; r++) {
        int row = r + rowOffset;
        QRect lineRect(0, topOffset + r * rowHeight, W, rowHeight);

        if (row % 16 == 0)
            p.fillRect(QRect(0, lineRect.y(), rowNumW_ + chnW_ * MAX_CHN, rowHeight),
                       Theme::C::downbeat);
        else if (row % 4 == 0)
            p.fillRect(QRect(0, lineRect.y(), rowNumW_ + chnW_ * MAX_CHN, rowHeight),
                       Theme::C::beat);

        // Edit cursor row
        if (row == eppos)
            p.fillRect(lineRect, Theme::C::editRow);

        p.setPen(Theme::C::textDim);
        p.drawText(QRect(0, lineRect.y(), rowNumW_, rowHeight),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString("%1").arg(row, 3, 16, QLatin1Char('0')).toUpper());

        for (int c = 0; c < MAX_CHN; c++) {
            int patnum = epnum[c];
            int plen = pattlen[patnum];
            int x = rowNumW_ + c * chnW_;
            QRect cellRect(x, lineRect.y(), chnW_, rowHeight);

            // Selection block overlay
            if (epmarkchn == c) {
                int lo = qMin(epmarkstart, epmarkend);
                int hi = qMax(epmarkstart, epmarkend);
                if (row >= lo && row <= hi) {
                    QColor sel(Theme::C::highlight);
                    sel.setAlpha(40);
                    p.fillRect(cellRect, sel);
                }
            }

            // Playback row highlight per channel
            if (playing) {
                int prow = chn[c].pattptr / 4;
                if (prow == row) p.fillRect(cellRect, Theme::C::playRow);
            }

            // Edit-column tint on active channel/row
            if (c == epchn && row == eppos) {
                int tx = x + colWidth;
                QRect colRect;
                if (epcolumn == 0)      colRect = QRect(tx, lineRect.y(), 3 * colWidth, rowHeight);
                else if (epcolumn <= 2) colRect = QRect(tx + 4 * colWidth, lineRect.y(), 2 * colWidth, rowHeight);
                else if (epcolumn == 3) colRect = QRect(tx + 7 * colWidth, lineRect.y(), colWidth, rowHeight);
                else                    colRect = QRect(tx + 8 * colWidth, lineRect.y(), 2 * colWidth, rowHeight);
                p.fillRect(colRect, QColor(Theme::C::highlight.red(),
                                           Theme::C::highlight.green(),
                                           Theme::C::highlight.blue(), 35));
            }

            if (row >= plen) {
                p.setPen(Theme::C::textDim);
                p.drawText(cellRect.adjusted(colWidth, 0, 0, 0),
                           Qt::AlignLeft | Qt::AlignVCenter, "---");
                continue;
            }

            const unsigned char *cell = &pattern[patnum][row * 4];
            unsigned char note = cell[0];
            unsigned char ins = cell[1];
            unsigned char cmd = cell[2];
            unsigned char param = cell[3];

            QString noteStr;
            if (note == REST)       noteStr = "...";
            else if (note == KEYOFF) noteStr = "===";
            else if (note == KEYON)  noteStr = "+++";
            else if (note >= FIRSTNOTE && note <= LASTNOTE)
                noteStr = notename[note - FIRSTNOTE];
            else noteStr = "...";

            QString insStr = (ins == 0) ? ".." :
                QString("%1").arg(ins, 2, 16, QLatin1Char('0')).toUpper();
            QString cmdStr = (cmd == 0 && param == 0) ? "..." :
                QString("%1%2").arg(cmd, 1, 16, QLatin1Char('0'))
                              .arg(param, 2, 16, QLatin1Char('0')).toUpper();

            int tx = x + colWidth;
            int ty = lineRect.y() + rowHeight - 5;
            p.setPen(note == REST || note == KEYOFF || note == KEYON || note < FIRSTNOTE
                     ? Theme::C::textDim : Theme::C::note);
            p.drawText(QPoint(tx, ty), noteStr);
            p.setPen(ins ? Theme::C::instr : Theme::C::textDim);
            p.drawText(QPoint(tx + 4 * colWidth, ty), insStr);
            p.setPen(cmd ? Theme::C::cmdDigit : Theme::C::textDim);
            p.drawText(QPoint(tx + 7 * colWidth, ty), QString(cmdStr[0]));
            p.setPen(cmd || param ? Theme::C::cmdParam : Theme::C::textDim);
            p.drawText(QPoint(tx + 8 * colWidth, ty), cmdStr.mid(1));
        }
    }
}
