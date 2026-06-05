#include "PatternView.h"

#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QMouseEvent>
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
extern CHN chn[MAX_CHN];
int isplaying(void);
void patterncommands(void);
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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    Theme::applyDarkPalette(viewport());
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setFocusPolicy(Qt::StrongFocus);
    updateScrollRange();
}

int PatternView::visibleRows() const {
    return viewport()->height() / rowHeight;
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
    // Keep the edit cursor row in view
    int rowOffset = verticalScrollBar()->value();
    int rows = visibleRows();
    if (rows > 0) {
        if (eppos < rowOffset) verticalScrollBar()->setValue(eppos);
        else if (eppos >= rowOffset + rows)
            verticalScrollBar()->setValue(eppos - rows + 1);
    }
    viewport()->update();
}

bool PatternView::event(QEvent *e) {
    // Let Tab/Backtab fall through to MainWindow's application shortcut
    // (cycle edit modes). Everything else: default handling.
    return QAbstractScrollArea::event(e);
}

void PatternView::keyPressEvent(QKeyEvent *e) {
    int sdlKey = qtKeyToSDLKeysym(e->key());
    QString txt = e->text();
    int asciiKey = 0;
    if (!txt.isEmpty()) {
        ushort u = txt.at(0).unicode();
        if (u < 128) asciiKey = u;
    }
    // gpattern's `key` is ASCII; `rawkey` is SDL keysym; `shiftpressed`
    // is set on EITHER shift or ctrl per gconsole.c.
    bool shiftLike = (e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) != 0;
    rawkey = sdlKey;
    key = asciiKey;
    shiftpressed = shiftLike ? 1 : 0;
    patterncommands();
    refresh();
    emit patternEdited();
}

void PatternView::mousePressEvent(QMouseEvent *e) {
    // Click → place edit cursor on the clicked row/channel.
    const int rowNumW = 5 * colWidth;
    const int chnW = 12 * colWidth;
    int x = e->pos().x() - rowNumW;
    int row = verticalScrollBar()->value() + e->pos().y() / rowHeight;
    if (row < 0) row = 0;
    if (row >= MAX_PATTROWS) row = MAX_PATTROWS - 1;
    eppos = row;
    if (x >= 0) {
        int c = x / chnW;
        if (c >= 0 && c < MAX_CHN) {
            epchn = c;
            epcolumn = 0;
        }
    }
    refresh();
    emit patternEdited();
}

void PatternView::paintEvent(QPaintEvent *) {
    QPainter p(viewport());
    p.setFont(font());

    const int rowOffset = verticalScrollBar()->value();

    // Layout: row# (4 chars) | per channel: "NNN II CP " ~ 11 chars + 1 gap
    const int rowNumW = 5 * colWidth;
    const int chnW = 12 * colWidth;
    const int vuH = 10;
    const int vuPad = 4;

    // OctaMED-style per-channel VU bars across the top, one per channel.
    unsigned char levels[3] = {0,0,0};
    sid_getlevels(levels);
    for (int c = 0; c < MAX_CHN; c++) {
        int x = rowNumW + c * chnW + 2;
        int w = chnW - 6;
        QRect frame(x, vuPad, w, vuH);
        p.fillRect(frame, Theme::C::vuBg);
        p.setPen(Theme::C::sep);
        p.drawRect(frame);
        int filled = (int)((double)levels[c] / 255.0 * (w - 2));
        if (chn[c].mute) {
            p.setPen(QColor(120, 60, 60));
            p.drawText(QPoint(x + 4, vuPad + vuH - 2), "MUTE");
        } else if (filled > 0) {
            // Gradient: green (0%) → yellow (60%) → red (90%)
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

    // Channel header row below VU bars
    const int headerY = vuPad * 2 + vuH;
    const int headerH = rowHeight;
    for (int c = 0; c < MAX_CHN; c++) {
        int x = rowNumW + c * chnW + 2;
        bool active = (c == epchn);
        p.setPen(active ? Theme::C::highlight : Theme::C::textDim);
        QFont hf = font();
        hf.setBold(active);
        p.setFont(hf);
        p.drawText(QPoint(x + 4, headerY + headerH - 5),
                   QString("Channel %1%2").arg(c + 1).arg(chn[c].mute ? " M" : ""));
    }
    p.setFont(font());
    // Header separator line
    p.setPen(Theme::C::sep);
    p.drawLine(0, headerY + headerH, viewport()->width(), headerY + headerH);

    const int topOffset = headerY + headerH + 1;

    // Recompute visible rows accounting for VU strip + header.
    const int rows = (viewport()->height() - topOffset) / rowHeight;

    const bool playing = isplaying() != 0;

    for (int r = 0; r < rows; r++) {
        int row = r + rowOffset;
        QRect lineRect(0, topOffset + r * rowHeight, viewport()->width(), rowHeight);

        // Beat / downbeat highlight
        if (row % 16 == 0) {
            p.fillRect(QRect(0, lineRect.y(), rowNumW + chnW * MAX_CHN, rowHeight), Theme::C::downbeat);
        } else if (row % 4 == 0) {
            p.fillRect(QRect(0, lineRect.y(), rowNumW + chnW * MAX_CHN, rowHeight), Theme::C::beat);
        }
        // Edit cursor row
        if (row == eppos) {
            p.fillRect(lineRect, Theme::C::editRow);
        }
        p.setPen(Theme::C::textDim);
        p.drawText(QRect(0, lineRect.y(), rowNumW, rowHeight),
                   Qt::AlignRight | Qt::AlignVCenter,
                   QString("%1").arg(row, 3, 16, QLatin1Char('0')).toUpper());

        for (int c = 0; c < MAX_CHN; c++) {
            int patnum = epnum[c];
            int plen = pattlen[patnum];
            int x = rowNumW + c * chnW;
            QRect cellRect(x, lineRect.y(), chnW, rowHeight);

            // Playback row highlight per channel
            if (playing) {
                int prow = chn[c].pattptr / 4;
                if (prow == row) p.fillRect(cellRect, Theme::C::playRow);
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
            if (note == REST) noteStr = "...";
            else if (note == KEYOFF) noteStr = "===";
            else if (note == KEYON) noteStr = "+++";
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
