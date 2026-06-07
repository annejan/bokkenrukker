#include "InstrumentQuickList.h"
#include "Theme.h"

#include <QBrush>
#include <QColor>
#include <cstring>

extern "C" {
#include "gcommon.h"
#include "gplay.h"
extern INSTR instr[MAX_INSTR];
extern int einum;
extern int song_channels;
extern int songinit;
}

InstrumentQuickList::InstrumentQuickList(QWidget *parent) : QListWidget(parent) {
    setMinimumWidth(140);
    setMaximumWidth(200);
    setFont(Theme::monoFont(10));
    QPalette pal = palette();
    pal.setColor(QPalette::Base, Theme::C::bgAlt);
    pal.setColor(QPalette::Text, Theme::C::text);
    pal.setColor(QPalette::Highlight, Theme::C::editRow);
    setPalette(pal);
    for (int i = 1; i < MAX_INSTR; i++) addItem(QString());
    refresh();
    connect(this, &QListWidget::currentRowChanged, this, &InstrumentQuickList::onRowChanged);

    // Flash polls chn[] at ~33 Hz. Decoupled from audio frame rate — visible
    // flash for any instrument that fired since the previous poll.
    flashTimer_.setInterval(30);
    connect(&flashTimer_, &QTimer::timeout, this, &InstrumentQuickList::tickFlash);
}

// Per-channel hot colours. Channels 0..2 = SID1 (primary RGB so blends are
// obvious), 3..5 = SID2 (secondaries — distinct from SID1 but still readable
// on the dark background).
static const QColor chanColor[6] = {
    QColor(255,  80,  80),  // ch1  red
    QColor( 80, 255,  80),  // ch2  green
    QColor( 80, 160, 255),  // ch3  blue
    QColor( 80, 220, 220),  // ch4  cyan
    QColor(220,  80, 220),  // ch5  magenta
    QColor(255, 200,  80),  // ch6  amber
};

void InstrumentQuickList::setBlinkEnabled(bool on) {
    blinkEnabled_ = on;
    if (on) {
        flashTimer_.start();
    } else {
        flashTimer_.stop();
        // Clear lingering tinted backgrounds.
        for (int i = 1; i < MAX_INSTR; i++) {
            for (int c = 0; c < 6; c++) flash_[i][c] = 0;
            if (auto *it = item(i - 1)) it->setBackground(QBrush());
        }
    }
}

void InstrumentQuickList::tickFlash() {
    // Bump flash for any instrument currently sounding on the active channels.
    // songinit guards against picking up stale chn[].instr while stopped.
    if (songinit != PLAY_STOPPED) {
        int nc = song_channels;
        if (nc <= 0 || nc > MAX_CHN) nc = MAX_CHN;
        // PortAudio runs at ~10 ms latency — chn[c].instr is close enough to
        // what's audible that we don't need a snapshot ring to compensate.
        for (int c = 0; c < nc; c++) {
            unsigned ins = chn[c].instr;
            if (ins >= 1 && ins < MAX_INSTR) flash_[ins][c] = 12;
        }
    }
    // Paint + decay. Sum weighted channel colours over the base for each row.
    QColor base = Theme::C::bgAlt;
    for (int i = 1; i < MAX_INSTR; i++) {
        auto *it = item(i - 1);
        if (!it) continue;
        float wSum = 0.f;
        float r = 0, g = 0, b = 0;
        for (int c = 0; c < 6; c++) {
            uint8_t f = flash_[i][c];
            if (!f) continue;
            float w = f / 12.0f;
            wSum += w;
            r += chanColor[c].red()   * w;
            g += chanColor[c].green() * w;
            b += chanColor[c].blue()  * w;
            flash_[i][c] = f - 1;
        }
        if (wSum <= 0.f) {
            // Repaint the base once after the last channel's flash hit zero;
            // skip otherwise to avoid setBackground churn on cold rows.
            if (it->background() != QBrush()) it->setBackground(QBrush(base));
            continue;
        }
        // Normalise the colour by total weight, then lerp from base towards
        // it by min(wSum,1) so a single faint channel stays subtle and many
        // concurrent channels saturate.
        float invW = 1.f / wSum;
        QColor hot(int(r * invW), int(g * invW), int(b * invW));
        float t = qMin(wSum, 1.0f);
        QColor mix(
            int(base.red()   * (1 - t) + hot.red()   * t),
            int(base.green() * (1 - t) + hot.green() * t),
            int(base.blue()  * (1 - t) + hot.blue()  * t));
        it->setBackground(QBrush(mix));
    }
}

void InstrumentQuickList::refresh() {
    updating_ = true;
    for (int i = 1; i < MAX_INSTR; i++) {
        char buf[MAX_INSTRNAMELEN + 1];
        std::memcpy(buf, instr[i].name, MAX_INSTRNAMELEN);
        buf[MAX_INSTRNAMELEN] = 0;
        QString name = QString::fromLocal8Bit(buf).trimmed();
        if (name.isEmpty()) name = "—";
        item(i - 1)->setText(QString("%1  %2")
            .arg(i, 2, 16, QLatin1Char('0')).toUpper().arg(name));
    }
    if (einum >= 1 && einum < MAX_INSTR)
        setCurrentRow(einum - 1);
    updating_ = false;
}

void InstrumentQuickList::onRowChanged(int row) {
    if (updating_ || row < 0) return;
    einum = row + 1;
    emit instrumentChosen();
}
