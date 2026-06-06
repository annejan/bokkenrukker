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

void InstrumentQuickList::setBlinkEnabled(bool on) {
    blinkEnabled_ = on;
    if (on) {
        flashTimer_.start();
    } else {
        flashTimer_.stop();
        // Clear any lingering tinted backgrounds.
        for (int i = 1; i < MAX_INSTR; i++) {
            flash_[i] = 0;
            if (auto *it = item(i - 1)) it->setBackground(QBrush());
        }
    }
}

void InstrumentQuickList::tickFlash() {
    // Bump flash for any instrument currently sounding on the active channels.
    // songinit guards against picking up stale chn[].instr while stopped.
    if (songinit != PLAY_STOPPED) {
        int ch = song_channels;
        if (ch <= 0 || ch > MAX_CHN) ch = MAX_CHN;
        for (int c = 0; c < ch; c++) {
            unsigned ins = chn[c].instr;
            if (ins >= 1 && ins < MAX_INSTR) flash_[ins] = 12; // ~360 ms tail
        }
    }
    // Paint + decay. Skip rows whose flash is already 0 to keep the work
    // cheap (no setBackground churn).
    QColor base   = Theme::C::bgAlt;
    QColor hot(255, 200, 80); // amber
    for (int i = 1; i < MAX_INSTR; i++) {
        uint8_t f = flash_[i];
        auto *it = item(i - 1);
        if (!it) continue;
        if (f == 0) continue;
        // Lerp base→hot by f/12.
        float t = f / 12.0f;
        QColor mix(
            int(base.red()   * (1 - t) + hot.red()   * t),
            int(base.green() * (1 - t) + hot.green() * t),
            int(base.blue()  * (1 - t) + hot.blue()  * t));
        it->setBackground(QBrush(mix));
        flash_[i] = f - 1;
        if (flash_[i] == 0) it->setBackground(QBrush(base));
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
