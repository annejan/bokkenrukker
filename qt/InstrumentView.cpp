#include "InstrumentView.h"
#include "Theme.h"
#include "UndoStack.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <cstring>

extern "C" {
#include "gcommon.h"
extern INSTR instr[MAX_INSTR];
extern int einum;
extern int epoctave;
extern int epchn;
extern int highestusedinstr;
void gototable(int t, int pos);
void playtestnote(int note, int ins, int chnnum);
void releasenote(int chnnum);
extern int editmode;
extern unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
extern unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
}

// ---------------------------------------------------------------------------
// ADSR envelope preview widget
// ---------------------------------------------------------------------------
class AdsrPreview : public QWidget {
public:
    explicit AdsrPreview(QWidget *parent) : QWidget(parent) {
        setMinimumSize(280, 140);
        Theme::applyDarkPalette(this);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Theme::C::bgAlt);
        setPalette(pal);
    }
    void setAdsr(int a, int d, int s, int r) {
        a_ = a; d_ = d; s_ = s; r_ = r;
        update();
    }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        int W = width(), H = height();
        int aw = 20 + a_ * 12;
        int dw = 20 + d_ * 10;
        int rw = 20 + r_ * 10;
        int baseY = H - 12;
        int peakY = 14;
        int sustY = baseY - (s_ * (baseY - peakY) / 15);
        QPoint p0(10, baseY);
        QPoint p1(p0.x() + aw, peakY);
        QPoint p2(p1.x() + dw, sustY);
        QPoint p3(p2.x() + 80, sustY);
        QPoint p4(p3.x() + rw, baseY);
        if (p4.x() > W - 8) p4.setX(W - 8);

        // Fill under envelope
        QPolygon poly;
        poly << QPoint(p0.x(), baseY) << p0 << p1 << p2 << p3 << p4
             << QPoint(p4.x(), baseY);
        QColor fill = Theme::C::highlight;
        fill.setAlpha(60);
        p.setBrush(fill);
        p.setPen(Qt::NoPen);
        p.drawPolygon(poly);

        // Envelope stroke
        p.setPen(QPen(Theme::C::highlight, 2));
        p.drawLine(p0, p1);
        p.drawLine(p1, p2);
        p.drawLine(p2, p3);
        p.drawLine(p3, p4);

        // Labels for phases
        p.setPen(Theme::C::textDim);
        QFont f = font();
        f.setPointSize(8);
        p.setFont(f);
        p.drawText(QPoint(p0.x(), baseY + 10), "0");
        p.drawText(QRect(p0.x(), peakY - 12, aw, 10),
                   Qt::AlignCenter, "A");
        p.drawText(QRect(p1.x(), peakY - 12, dw, 10),
                   Qt::AlignCenter, "D");
        p.drawText(QRect(p2.x(), sustY - 12, p3.x() - p2.x(), 10),
                   Qt::AlignCenter, "S");
        p.drawText(QRect(p3.x(), sustY - 12, rw, 10),
                   Qt::AlignCenter, "R");
    }
private:
    int a_ = 0, d_ = 0, s_ = 0, r_ = 0;
};

// ---------------------------------------------------------------------------
// Wavetable preview strip
// ---------------------------------------------------------------------------
class WavetablePreview : public QWidget {
public:
    explicit WavetablePreview(QWidget *parent) : QWidget(parent) {
        setMinimumHeight(28);
        Theme::applyDarkPalette(this);
        QPalette pal = palette();
        pal.setColor(QPalette::Window, Theme::C::bgAlt);
        setPalette(pal);
    }
    void setStart(int s) { start_ = s; update(); }
protected:
    void paintEvent(QPaintEvent *) override {
        QPainter p(this);
        const int N = 24; // show 24 wavetable steps
        int W = width(), H = height();
        int cellW = W / N;
        for (int i = 0; i < N; i++) {
            int idx = start_ + i - 1;
            if (idx < 0 || idx >= MAX_TABLELEN) continue;
            unsigned char L = ltable[WTBL][idx];
            QColor c = Theme::C::textDim;
            if (L == 0xff) c = Theme::C::rst;
            else if (L >= 0xf0) c = Theme::C::cmdDigit;
            else if (L >= 0xe0) c = Theme::C::textDim;
            else if (L >= 0x80) c = QColor(0xC0, 0x70, 0xC0);
            else if (L >= 0x40) c = QColor(0x6F, 0xA6, 0xCE);
            else if (L >= 0x20) c = QColor(0xE0, 0xA8, 0x4A);
            else if (L >= 0x10) c = QColor(0xA1, 0xC1, 0x81);
            else if (L >= 0x01) c = Theme::C::transpose;
            // Reserve bottom strip for the legend; cells live above it.
            const int legendH = 12;
            int cellsTop = 2;
            int cellsH = qMax(8, H - legendH - 2);
            QRect r(i * cellW + 1, cellsTop, cellW - 2, cellsH);
            p.fillRect(r, c);
            p.setPen(Theme::C::sep);
            p.drawRect(r);
        }
        p.setPen(Theme::C::textDim);
        QFont f = font();
        f.setPointSize(8);
        p.setFont(f);
        // Bottom-aligned legend, in its own reserved strip (no cell overlap)
        p.drawText(QRect(4, H - 12, W - 8, 12),
                   Qt::AlignVCenter,
                   "noise=purple  pulse=blue  saw=orange  tri=green  cmd=red  jump=orange");
    }
private:
    int start_ = 1;
};

// ---------------------------------------------------------------------------
// Helper: hex spinbox 0..255
// ---------------------------------------------------------------------------
static QSpinBox *makeHexSpin(QWidget *parent) {
    auto *s = new QSpinBox(parent);
    s->setRange(0, 255);
    s->setDisplayIntegerBase(16);
    s->setPrefix("$");
    QFont f = Theme::monoFont(11);
    s->setFont(f);
    s->setMinimumWidth(80);
    return s;
}

static QSpinBox *makeNybbleSpin(QWidget *parent) {
    auto *s = new QSpinBox(parent);
    s->setRange(0, 15);
    s->setDisplayIntegerBase(16);
    QFont f = Theme::monoFont(11);
    s->setFont(f);
    s->setMinimumWidth(60);
    return s;
}

// ---------------------------------------------------------------------------

InstrumentView::InstrumentView(QWidget *parent) : QWidget(parent) {
    Theme::applyDarkPalette(this);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    // Left: instrument list -----------------------------------------------
    list_ = new QListWidget(this);
    list_->setMinimumWidth(200);
    list_->setMaximumWidth(240);
    list_->setFont(Theme::monoFont(11));
    QPalette lp = list_->palette();
    lp.setColor(QPalette::Base, Theme::C::bgAlt);
    lp.setColor(QPalette::Text, Theme::C::text);
    lp.setColor(QPalette::Highlight, Theme::C::editRow);
    list_->setPalette(lp);
    connect(list_, &QListWidget::currentRowChanged, this, &InstrumentView::onListChanged);
    root->addWidget(list_);

    // Center: editor form --------------------------------------------------
    auto *center = new QVBoxLayout();
    center->setSpacing(8);

    auto *header = new QLabel("Instrument", this);
    QFont hf = header->font(); hf.setPointSize(13); hf.setBold(true);
    header->setFont(hf);
    QPalette hp = header->palette();
    hp.setColor(QPalette::WindowText, Theme::C::highlight);
    header->setPalette(hp);
    center->addWidget(header);

    auto *nameRow = new QHBoxLayout();
    auto *nameLbl = new QLabel("Name:", this);
    name_ = new QLineEdit(this);
    name_->setMaxLength(MAX_INSTRNAMELEN - 1);
    name_->setFont(Theme::monoFont(11));
    name_->setToolTip("Display name (max 16 chars). Saved with the .sng and "
                      "shown in the instrument list.");
    nameRow->addWidget(nameLbl);
    nameRow->addWidget(name_, 1);
    center->addLayout(nameRow);
    connect(name_, &QLineEdit::textEdited, this, &InstrumentView::onNameChanged);

    // Preset envelope picker — sets ADSR + 1stFrame Wave + name to a sensible
    // default for a common instrument archetype. Wavetable / pulse / filter
    // pointers are NOT touched, so existing wavetable programs keep working.
    auto *presetRow = new QHBoxLayout();
    auto *presetLbl = new QLabel("Preset:", this);
    presetBox_ = new QComboBox(this);
    presetBox_->setToolTip("Apply a starter envelope for a common instrument "
                           "archetype. Only ADSR / name / 1stFrame Wave change; "
                           "wavetable pointers stay where they are.");
    presetBox_->addItem("(choose a preset…)");
    presetBox_->addItem("Bass — tight pulse");
    presetBox_->addItem("Bass — triangle, soft");
    presetBox_->addItem("Lead — sawtooth");
    presetBox_->addItem("Pluck — fast decay");
    presetBox_->addItem("Pad — slow attack, sustain");
    presetBox_->addItem("Brass — punchy attack");
    presetBox_->addItem("Organ — full sustain, no release");
    presetBox_->addItem("Bell — instant attack, long decay");
    presetBox_->addItem("Strings — soft attack, full sustain");
    presetBox_->addItem("Drum kit: Kick");
    presetBox_->addItem("Drum kit: Snare");
    presetBox_->addItem("Drum kit: Closed hat");
    presetBox_->addItem("Drum kit: Open hat");
    auto *applyBtn = new QPushButton("Apply", this);
    applyBtn->setToolTip("Overwrite this instrument's envelope with the "
                         "selected preset.");
    connect(applyBtn, &QPushButton::clicked, this,
            [this]{ applyPreset(presetBox_->currentIndex()); });
    presetRow->addWidget(presetLbl);
    presetRow->addWidget(presetBox_, 1);
    presetRow->addWidget(applyBtn);
    center->addLayout(presetRow);

    auto *envBox = new QGroupBox("Envelope (ADSR)", this);
    envBox->setToolTip("SID envelope shape. $0 is fastest, $F is slowest. "
                       "Attack 0 can make the first wavetable row silent.");
    auto *envForm = new QFormLayout(envBox);
    attack_ = makeNybbleSpin(envBox);
    attack_->setToolTip("Attack rate ($0 fastest = 2ms, $F slowest = 8s). "
                        "Set high on pads/strings, low on bass/lead.");
    decay_ = makeNybbleSpin(envBox);
    decay_->setToolTip("Decay rate from peak to sustain level. "
                       "$0 = 6ms, $F = 24s. Controls how fast the note 'rings down'.");
    sustain_ = makeNybbleSpin(envBox);
    sustain_->setToolTip("Sustain level ($0 silent, $F loudest). "
                         "Held while gate is on. Set to 0 for pluck/percussive.");
    release_ = makeNybbleSpin(envBox);
    release_->setToolTip("Release rate after key-off ($0 fastest, $F slowest). "
                         "Watch out: A=0, R=1 can ADSR-bug on some chips.");
    envForm->addRow("Attack", attack_);
    envForm->addRow("Decay", decay_);
    envForm->addRow("Sustain", sustain_);
    envForm->addRow("Release", release_);
    connect(attack_, QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onAdChanged);
    connect(decay_,  QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onAdChanged);
    connect(sustain_,QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onSrChanged);
    connect(release_,QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onSrChanged);
    center->addWidget(envBox);

    auto *tblBox = new QGroupBox("Table pointers", this);
    tblBox->setToolTip("Start-position pointers into the four shared tables. "
                       "$00 means 'no execution' (except Wave, where $00 is unusable).");
    auto *tblForm = new QFormLayout(tblBox);
    wave_ = makeHexSpin(tblBox);
    wave_->setToolTip("Wavetable start step. Drives waveform + arpeggio + "
                      "drum programs. Must be ≥ $01.");
    pulse_ = makeHexSpin(tblBox);
    pulse_->setToolTip("Pulsetable start step. $00 = leave pulse execution "
                       "alone. Used for PWM sweeps.");
    filter_ = makeHexSpin(tblBox);
    filter_->setToolTip("Filtertable start step. $00 = leave filter alone. "
                        "Only one channel should drive the filter at a time.");
    vibParam_ = makeHexSpin(tblBox);
    vibParam_->setToolTip("Speedtable index for instrument vibrato. $00 = no "
                          "vibrato. Vibrato delay sets how many ticks before it starts.");

    auto wrapJump = [this, tblBox](QSpinBox *s, void (InstrumentView::*slot)()) {
        auto *row = new QHBoxLayout();
        row->addWidget(s);
        auto *btn = new QPushButton("→ table", tblBox);
        btn->setMaximumWidth(80);
        btn->setToolTip("Open the Tables editor and jump the cursor to the "
                        "row this pointer references — quick way to find / "
                        "edit the wavetable / pulsetable / filtertable "
                        "program this instrument runs.");
        connect(btn, &QPushButton::clicked, this, slot);
        row->addWidget(btn);
        return row;
    };
    tblForm->addRow("Wavetable Pos",   wrapJump(wave_,   &InstrumentView::onGotoWaveTable));
    tblForm->addRow("Pulsetable Pos",  wrapJump(pulse_,  &InstrumentView::onGotoPulseTable));
    tblForm->addRow("Filtertable Pos", wrapJump(filter_, &InstrumentView::onGotoFilterTable));
    tblForm->addRow("Vibrato Param",   vibParam_);
    connect(wave_,    QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onWtChanged);
    connect(pulse_,   QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onPtChanged);
    connect(filter_,  QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onFtChanged);
    connect(vibParam_,QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onVibParamChanged);
    center->addWidget(tblBox);

    auto *miscBox = new QGroupBox("Misc", this);
    auto *miscForm = new QFormLayout(miscBox);
    vibDelay_ = makeHexSpin(miscBox);
    vibDelay_->setToolTip("Ticks before instrument vibrato kicks in. "
                          "$00 disables vibrato entirely.");
    gateTimer_ = makeHexSpin(miscBox);
    gateTimer_->setToolTip("Hard-restart / gate-off length in ticks. "
                           "Must be < tempo or playback halts. "
                           "Bit $80 disables hard restart, $40 disables gate-off.");
    firstWave_ = makeHexSpin(miscBox);
    firstWave_->setToolTip("Waveform written on the very first frame of a note. "
                           "$09 = gate+test (default). $00/$FE/$FF special-case "
                           "the gate behaviour for legato instruments.");
    miscForm->addRow("Vibrato Delay", vibDelay_);
    miscForm->addRow("HR/Gate Timer", gateTimer_);
    miscForm->addRow("1stFrame Wave", firstWave_);
    connect(vibDelay_, QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onVibDelayChanged);
    connect(gateTimer_,QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onGateChanged);
    connect(firstWave_,QOverload<int>::of(&QSpinBox::valueChanged), this, &InstrumentView::onFirstWaveChanged);
    center->addWidget(miscBox);

    auto *btnRow = new QHBoxLayout();
    auto *testBtn = new QPushButton("Play test note", this);
    testBtn->setToolTip("Play C of the current octave through this instrument "
                        "on the current channel.");
    auto *stopBtn = new QPushButton("Silence", this);
    stopBtn->setToolTip("Release the test note on the current channel.");
    connect(testBtn, &QPushButton::clicked, this, &InstrumentView::onTestNote);
    connect(stopBtn, &QPushButton::clicked, this, &InstrumentView::onSilenceTestNote);
    btnRow->addWidget(testBtn);
    btnRow->addWidget(stopBtn);
    btnRow->addStretch();
    center->addLayout(btnRow);

    center->addStretch();
    root->addLayout(center, 1);

    // Right: previews ------------------------------------------------------
    auto *right = new QVBoxLayout();
    right->setSpacing(8);

    auto *envHdr = new QLabel("ADSR shape", this);
    QFont eh = envHdr->font(); eh.setBold(true);
    envHdr->setFont(eh);
    right->addWidget(envHdr);

    adsr_ = new AdsrPreview(this);
    right->addWidget(adsr_);

    auto *waveHdr = new QLabel("Wavetable program (first 24 steps)", this);
    waveHdr->setFont(eh);
    right->addWidget(waveHdr);

    wavePrev_ = new WavetablePreview(this);
    wavePrev_->setMinimumHeight(40);
    right->addWidget(wavePrev_);

    // Compact summary card — fits in its own bordered frame, kept short.
    summary_ = new QLabel(this);
    summary_->setWordWrap(true);
    summary_->setTextFormat(Qt::RichText);
    summary_->setMinimumHeight(80);
    summary_->setContentsMargins(10, 8, 10, 8);
    summary_->setAutoFillBackground(true);
    QPalette sp = summary_->palette();
    sp.setColor(QPalette::Window, Theme::C::bgAlt);
    sp.setColor(QPalette::WindowText, Theme::C::text);
    summary_->setPalette(sp);
    right->addWidget(summary_);

    right->addStretch();
    root->addLayout(right, 1);

    // Populate list
    for (int i = 1; i < MAX_INSTR; i++) list_->addItem(QString());
    refresh();
}

void InstrumentView::refresh() {
    updating_ = true;
    // Refresh list contents — names may have changed
    for (int i = 1; i < MAX_INSTR; i++) {
        char buf[MAX_INSTRNAMELEN + 1];
        std::memcpy(buf, instr[i].name, MAX_INSTRNAMELEN);
        buf[MAX_INSTRNAMELEN] = 0;
        QString name = QString::fromLocal8Bit(buf).trimmed();
        if (name.isEmpty()) name = "—";
        list_->item(i - 1)->setText(
            QString("%1  %2")
                .arg(i, 2, 16, QLatin1Char('0')).toUpper()
                .arg(name));
    }
    if (einum < 1) einum = 1;
    list_->setCurrentRow(einum - 1);
    readFromGlobals();
    updating_ = false;
}

void InstrumentView::readFromGlobals() {
    updating_ = true;
    INSTR &ins = instr[einum];
    char buf[MAX_INSTRNAMELEN + 1];
    std::memcpy(buf, ins.name, MAX_INSTRNAMELEN);
    buf[MAX_INSTRNAMELEN] = 0;
    name_->setText(QString::fromLocal8Bit(buf));
    attack_->setValue((ins.ad >> 4) & 0xf);
    decay_->setValue(ins.ad & 0xf);
    sustain_->setValue((ins.sr >> 4) & 0xf);
    release_->setValue(ins.sr & 0xf);
    wave_->setValue(ins.ptr[WTBL]);
    pulse_->setValue(ins.ptr[PTBL]);
    filter_->setValue(ins.ptr[FTBL]);
    vibParam_->setValue(ins.ptr[STBL]);
    vibDelay_->setValue(ins.vibdelay);
    gateTimer_->setValue(ins.gatetimer);
    firstWave_->setValue(ins.firstwave);
    adsr_->setAdsr(attack_->value(), decay_->value(),
                   sustain_->value(), release_->value());
    wavePrev_->setStart(ins.ptr[WTBL]);

    QString fwDesc;
    if (ins.firstwave == 0x09)      fwDesc = "gate+test (std)";
    else if (ins.firstwave == 0x00) fwDesc = "legato (no gate change)";
    else if (ins.firstwave == 0xfe) fwDesc = "gate off";
    else if (ins.firstwave == 0xff) fwDesc = "gate on";
    else                            fwDesc = "custom";

    bool hrDisabled = (ins.gatetimer & 0x80) != 0;
    bool gateOffDisabled = (ins.gatetimer & 0x40) != 0;
    int gateTicks = ins.gatetimer & 0x3f;
    QString gateDesc = QString("%1 ticks").arg(gateTicks);
    if (hrDisabled) gateDesc += ", no HR";
    if (gateOffDisabled) gateDesc += ", no gate-off";

    summary_->setText(QString(
        "<b style='color:#FFD93D'>Instrument $%1</b><br>"
        "ADSR = A%2 D%3 S%4 R%5"
        "<br>1stFrame Wave $%6 — %7"
        "<br>Gate/HR timer $%8 — %9")
        .arg(einum, 2, 16, QLatin1Char('0'))
        .arg(attack_->value(), 1, 16).arg(decay_->value(), 1, 16)
        .arg(sustain_->value(), 1, 16).arg(release_->value(), 1, 16)
        .arg(ins.firstwave, 2, 16, QLatin1Char('0')).arg(fwDesc)
        .arg(ins.gatetimer, 2, 16, QLatin1Char('0')).arg(gateDesc)
        .toUpper());
    updating_ = false;
}

void InstrumentView::onListChanged(int row) {
    if (updating_ || row < 0) return;
    einum = row + 1;
    readFromGlobals();
    emit edited();
}

void InstrumentView::onNameChanged(const QString &t) {
    if (updating_) return;
    QByteArray before = captureSongSnapshot();
    QByteArray ba = t.toLocal8Bit();
    INSTR &ins = instr[einum];
    int n = qMin<int>(MAX_INSTRNAMELEN - 1, ba.size());
    std::memcpy(ins.name, ba.constData(), n);
    for (int i = n; i < MAX_INSTRNAMELEN; i++) ins.name[i] = 0;
    char buf[MAX_INSTRNAMELEN + 1];
    std::memcpy(buf, ins.name, MAX_INSTRNAMELEN);
    buf[MAX_INSTRNAMELEN] = 0;
    QString name = QString::fromLocal8Bit(buf).trimmed();
    if (name.isEmpty()) name = "—";
    list_->item(einum - 1)->setText(
        QString("%1  %2").arg(einum, 2, 16, QLatin1Char('0')).toUpper().arg(name));
    pushEditIfChanged(this, std::move(before), "Instrument name");
    emit edited();
}

void InstrumentView::writeAd() {
    instr[einum].ad = (attack_->value() << 4) | decay_->value();
}
void InstrumentView::writeSr() {
    instr[einum].sr = (sustain_->value() << 4) | release_->value();
}

void InstrumentView::onAdChanged(int) {
    if (updating_) return;
    QByteArray before = captureSongSnapshot();
    writeAd();
    adsr_->setAdsr(attack_->value(), decay_->value(),
                   sustain_->value(), release_->value());
    pushEditIfChanged(this, std::move(before), "Instrument ADSR");
    emit edited();
}
void InstrumentView::onSrChanged(int) {
    if (updating_) return;
    QByteArray before = captureSongSnapshot();
    writeSr();
    adsr_->setAdsr(attack_->value(), decay_->value(),
                   sustain_->value(), release_->value());
    pushEditIfChanged(this, std::move(before), "Instrument ADSR");
    emit edited();
}
void InstrumentView::onWtChanged(int v) {
    if (updating_) return;
    QByteArray before = captureSongSnapshot();
    instr[einum].ptr[WTBL] = v;
    wavePrev_->setStart(v);
    pushEditIfChanged(this, std::move(before), "Instrument param");
    emit edited();
}
void InstrumentView::onPtChanged(int v) {
    if (updating_) return;
    QByteArray b = captureSongSnapshot(); instr[einum].ptr[PTBL] = v;
    pushEditIfChanged(this, std::move(b), "Instrument param"); emit edited();
}
void InstrumentView::onFtChanged(int v) {
    if (updating_) return;
    QByteArray b = captureSongSnapshot(); instr[einum].ptr[FTBL] = v;
    pushEditIfChanged(this, std::move(b), "Instrument param"); emit edited();
}
void InstrumentView::onVibParamChanged(int v) {
    if (updating_) return;
    QByteArray b = captureSongSnapshot(); instr[einum].ptr[STBL] = v;
    pushEditIfChanged(this, std::move(b), "Instrument param"); emit edited();
}
void InstrumentView::onVibDelayChanged(int v) {
    if (updating_) return;
    QByteArray b = captureSongSnapshot(); instr[einum].vibdelay = v;
    pushEditIfChanged(this, std::move(b), "Instrument param"); emit edited();
}
void InstrumentView::onGateChanged(int v) {
    if (updating_) return;
    QByteArray b = captureSongSnapshot(); instr[einum].gatetimer = v;
    pushEditIfChanged(this, std::move(b), "Instrument param"); emit edited();
}
void InstrumentView::onFirstWaveChanged(int v) {
    if (updating_) return;
    QByteArray b = captureSongSnapshot(); instr[einum].firstwave = v;
    pushEditIfChanged(this, std::move(b), "Instrument param"); emit edited();
}

void InstrumentView::onGotoWaveTable() {
    gototable(WTBL, instr[einum].ptr[WTBL] > 0 ? instr[einum].ptr[WTBL] - 1 : 0);
    editmode = 3; // EDIT_TABLES
    emit edited();
}
void InstrumentView::onGotoPulseTable() {
    gototable(PTBL, instr[einum].ptr[PTBL] > 0 ? instr[einum].ptr[PTBL] - 1 : 0);
    editmode = 3;
    emit edited();
}
void InstrumentView::onGotoFilterTable() {
    gototable(FTBL, instr[einum].ptr[FTBL] > 0 ? instr[einum].ptr[FTBL] - 1 : 0);
    editmode = 3;
    emit edited();
}

void InstrumentView::onTestNote() {
    int note = FIRSTNOTE + epoctave * 12; // C of current octave
    playtestnote(note, einum, epchn);
}
void InstrumentView::onSilenceTestNote() {
    releasenote(epchn);
}

// Preset starter envelopes from docs/sid-composition.md.
// Tuple: name (display), AD byte, SR byte, 1stFrame, "label written into ins.name".
namespace {
struct Preset {
    const char *label;
    unsigned char ad;
    unsigned char sr;
    unsigned char firstwave;
    const char *insName;
};
static const Preset kPresets[] = {
    {"",                       0,    0,    0x09, ""},          // index 0 placeholder
    {"Bass — tight pulse",     0x09, 0x00, 0x09, "Bass"},
    {"Bass — triangle soft",   0x0A, 0xFC, 0x09, "Bass tri"},
    {"Lead — sawtooth",        0x08, 0x85, 0x09, "Lead"},
    {"Pluck — fast decay",     0x00, 0xC0, 0x09, "Pluck"},
    {"Pad — slow, sustain",    0x84, 0xC5, 0x09, "Pad"},
    {"Brass — punchy",         0x2A, 0xF4, 0x09, "Brass"},
    {"Organ — full sustain",   0x00, 0xF0, 0x09, "Organ"},
    {"Bell — long decay",      0x0B, 0x00, 0x09, "Bell"},
    {"Strings — soft sus",     0x88, 0xCA, 0x09, "Strings"},
    // Drum kit — sensible ADSR seeds. Wavetable still needs to be wired by
    // hand (kick: pitch slide + noise burst; snare: noise+pulse; hat: noise).
    // docs/sid-composition.md §3 covers the wavetable programs.
    {"Drum kit: Kick",         0x00, 0x09, 0x09, "Kick"},
    {"Drum kit: Snare",        0x00, 0x05, 0x09, "Snare"},
    {"Drum kit: Closed hat",   0x00, 0x02, 0x09, "Hat-cl"},
    {"Drum kit: Open hat",     0x00, 0x07, 0x09, "Hat-op"},
};
}

void InstrumentView::applyPreset(int index) {
    if (index <= 0 || index >= (int)(sizeof(kPresets) / sizeof(kPresets[0]))) return;
    QByteArray before = captureSongSnapshot();
    const Preset &p = kPresets[index];
    INSTR &ins = instr[einum];
    ins.ad = p.ad;
    ins.sr = p.sr;
    ins.firstwave = p.firstwave;
    // Only rename if the slot is empty or already a default placeholder.
    char nameBuf[MAX_INSTRNAMELEN + 1];
    std::memcpy(nameBuf, ins.name, MAX_INSTRNAMELEN);
    nameBuf[MAX_INSTRNAMELEN] = 0;
    QString cur = QString::fromLocal8Bit(nameBuf).trimmed();
    if (cur.isEmpty()) {
        std::strncpy(ins.name, p.insName, MAX_INSTRNAMELEN - 1);
        ins.name[MAX_INSTRNAMELEN - 1] = 0;
    }
    pushEditIfChanged(this, std::move(before), "Apply preset");
    refresh();
    emit edited();
}

