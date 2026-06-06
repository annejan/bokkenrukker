#include "TablesView.h"
#include "Theme.h"
#include "SdlKeyMap.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QTableView>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QKeyEvent>
#include <QMenu>

extern "C" {
#include "gcommon.h"
extern unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
extern unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
extern int etnum;
extern int etpos;
extern int etcolumn;
extern int etview[MAX_TABLES];
extern int etlock;
extern INSTR instr[MAX_INSTR];
extern int key;
extern int rawkey;
extern int shiftpressed;
void tablecommands(void);
}

static const char *tableName[4] = {"Wave", "Pulse", "Filter", "Speed"};
static const QColor tableTint[4] = {
    QColor(0xA1, 0xC1, 0x81),
    QColor(0x6F, 0xA6, 0xCE),
    QColor(0xD8, 0x9B, 0x8C),
    QColor(0xE0, 0xC1, 0x6E)
};

// ---------------------------------------------------------------------------
// Cell decoder (kept from previous incarnation)
// ---------------------------------------------------------------------------
static QString decodeCell(int t, unsigned char L, unsigned char R) {
    auto hex = [](unsigned v, int w = 2) {
        return QString("%1").arg(v, w, 16, QLatin1Char('0')).toUpper();
    };
    QString line;
    if (t == 0) {
        if (L == 0x00) line = "no change";
        else if (L >= 0x01 && L <= 0x0F) line = QString("delay %1 frames").arg(L);
        else if (L >= 0xE0 && L <= 0xEF) line = QString("inaudible $%1").arg(hex(L - 0xE0));
        else if (L == 0xFF) line = R == 0 ? "stop" : QString("jump to step $%1").arg(hex(R));
        else if (L >= 0xF0 && L <= 0xFE) line = QString("execute cmd %1XY param $%2")
                                                .arg(L - 0xF0, 1, 16).arg(hex(R));
        else {
            QStringList bits;
            if (L & 0x80) bits << "noise";
            if (L & 0x40) bits << "pulse";
            if (L & 0x20) bits << "saw";
            if (L & 0x10) bits << "tri";
            if (L & 0x08) bits << "test";
            if (L & 0x04) bits << "ring";
            if (L & 0x02) bits << "sync";
            if (L & 0x01) bits << "gate";
            line = QString("wave $%1 [%2]").arg(hex(L)).arg(bits.join('+'));
        }
        QString rs;
        if (R == 0x80) rs = "hold freq";
        else if (R >= 0x81) rs = QString("abs note %1").arg(R - 0x81);
        else if (R >= 0x60) rs = QString("neg rel %1").arg(R - 0x60);
        else rs = QString("rel +%1").arg(R);
        line += "    note: " + rs;
    } else if (t == 1) {
        if (L == 0xFF) line = R == 0 ? "stop" : QString("jump to step $%1").arg(hex(R));
        else if (L >= 0x80) {
            int pw = ((L & 0x0F) << 8) | R;
            line = QString("set pulse width $%1").arg(hex(pw, 3));
        } else {
            int spd = (signed char)R;
            line = QString("modulate %1 ticks @ %2%3").arg(L)
                   .arg(spd >= 0 ? "+" : "").arg(spd);
        }
    } else if (t == 2) {
        if (L == 0x00) line = QString("set cutoff $%1").arg(hex(R));
        else if (L == 0xFF) line = R == 0 ? "stop" : QString("jump to step $%1").arg(hex(R));
        else if (L >= 0x80) {
            const char *pass = "??";
            switch (L & 0xF0) {
                case 0x80: pass = "off";      break;
                case 0x90: pass = "lowpass";  break;
                case 0xA0: pass = "bandpass"; break;
                case 0xB0: pass = "low+band"; break;
                case 0xC0: pass = "highpass"; break;
                case 0xD0: pass = "low+high"; break;
                case 0xE0: pass = "high+bnd"; break;
                case 0xF0: pass = "all-pass"; break;
            }
            int res = (R >> 4) & 0xf;
            int mask = R & 0xf;
            line = QString("filter %1, res=$%2, ch-mask=%3")
                   .arg(pass).arg(res, 1, 16).arg(mask, 1, 16);
        } else {
            int spd = (signed char)R;
            line = QString("modulate %1 ticks @ %2%3").arg(L)
                   .arg(spd >= 0 ? "+" : "").arg(spd);
        }
    } else {
        bool noteIndep = (L & 0x80) != 0;
        if (noteIndep) {
            line = QString("note-indep, divisor %1").arg(L & 0x7f);
        } else {
            line = QString("vib spd=%1 depth=%2  porta=%3  funk evn=%4 odd=%5")
                   .arg(L, 1, 16).arg(R, 1, 16)
                   .arg(((unsigned)L << 8) | R, 4, 16, QLatin1Char('0'))
                   .arg(L, 2, 16, QLatin1Char('0'))
                   .arg(R, 2, 16, QLatin1Char('0')).toUpper();
        }
    }
    return line;
}

// ---------------------------------------------------------------------------
// Model — columns: 0 = index, 1 = left byte, 2 = right byte
// ---------------------------------------------------------------------------
SidTableModel::SidTableModel(int tableIndex, QObject *parent)
    : QAbstractTableModel(parent), t_(tableIndex) {}

int SidTableModel::rowCount(const QModelIndex &) const { return MAX_TABLELEN; }
int SidTableModel::columnCount(const QModelIndex &) const { return 3; }

QVariant SidTableModel::data(const QModelIndex &i, int role) const {
    if (!i.isValid()) return {};
    int row = i.row();
    int col = i.column();
    unsigned char L = ltable[t_][row];
    unsigned char R = rtable[t_][row];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (col == 0) return QString("%1").arg(row + 1, 2, 16, QLatin1Char('0')).toUpper();
        if (col == 1) return QString("%1").arg(L, 2, 16, QLatin1Char('0')).toUpper();
        if (col == 2) return QString("%1").arg(R, 2, 16, QLatin1Char('0')).toUpper();
    }
    if (role == Qt::TextAlignmentRole) return int(Qt::AlignCenter);
    if (role == Qt::FontRole) return Theme::monoFont(11);

    if (role == Qt::ForegroundRole) {
        if (col == 0) return QBrush(Theme::C::textDim);
        if (col == 1) return QBrush(L ? Theme::C::cmdDigit : Theme::C::textDim);
        if (col == 2) return QBrush(R ? Theme::C::cmdParam : Theme::C::textDim);
    }
    return {};
}

QVariant SidTableModel::headerData(int section, Qt::Orientation o, int role) const {
    if (role != Qt::DisplayRole) return {};
    if (o == Qt::Horizontal) {
        switch (section) {
            case 0: return QString("Idx");
            case 1: return QString("L");
            case 2: return QString("R");
        }
    }
    return {};
}

bool SidTableModel::setData(const QModelIndex &i, const QVariant &v, int role) {
    if (role != Qt::EditRole || !i.isValid()) return false;
    int col = i.column(); int row = i.row();
    if (col == 0) return false; // index column not editable
    bool ok = false;
    int val = v.toString().toInt(&ok, 16);
    if (!ok || val < 0 || val > 255) return false;
    if (col == 1) ltable[t_][row] = (unsigned char)val;
    else if (col == 2) rtable[t_][row] = (unsigned char)val;
    emit dataChanged(i, i);
    return true;
}

Qt::ItemFlags SidTableModel::flags(const QModelIndex &i) const {
    if (!i.isValid()) return Qt::NoItemFlags;
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (i.column() != 0) f |= Qt::ItemIsEditable;
    return f;
}

void SidTableModel::refresh() {
    beginResetModel();
    endResetModel();
}

// ---------------------------------------------------------------------------
// Delegate paints semantic backgrounds + cursor
// ---------------------------------------------------------------------------
class TableCellDelegate : public QStyledItemDelegate {
public:
    TableCellDelegate(int tableIdx, QObject *parent = nullptr)
        : QStyledItemDelegate(parent), t_(tableIdx) {}
    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &i) const override {
        int row = i.row();
        unsigned char L = ltable[t_][row];
        QColor bg = Theme::C::bgBase;
        // Empty rows dim
        if (L == 0 && rtable[t_][row] == 0) bg = Theme::C::bgBase;
        else if (L == 0xFF) bg = QColor(70, 30, 25);            // jump
        else if (L >= 0xF0 && t_ == 0) bg = QColor(70, 45, 20); // wave-cmd exec
        else if (L >= 0x80 && t_ == 0) bg = QColor(50, 25, 50); // noise
        else if (L >= 0x40 && t_ == 0) bg = QColor(20, 40, 60); // pulse
        else if (L >= 0x20 && t_ == 0) bg = QColor(60, 45, 20); // saw
        else if (L >= 0x10 && t_ == 0) bg = QColor(30, 50, 30); // tri
        else if (L >= 0x01 && t_ == 0 && L <= 0x0F) bg = QColor(20, 30, 50); // delay
        else if (t_ == 1 && L >= 0x80 && L < 0xFF) bg = QColor(20, 40, 55); // pulse-pw
        else if (t_ == 2 && L >= 0x80 && L < 0xFF) bg = QColor(55, 35, 30); // filter params
        if (row % 4 == 0 && bg == Theme::C::bgBase) bg = Theme::C::beat;

        p->fillRect(opt.rect, bg);
        // Cursor highlight (matches the global etnum / etpos)
        if (t_ == etnum && row == etpos) {
            p->fillRect(opt.rect, Theme::C::editRow);
        }
        QStyledItemDelegate::paint(p, opt, i);
        if (t_ == etnum && row == etpos) {
            p->setPen(QPen(Theme::C::highlight, 1));
            p->drawRect(opt.rect.adjusted(0, 0, -1, -1));
        }
    }
private:
    int t_;
};

// ---------------------------------------------------------------------------
// View
// ---------------------------------------------------------------------------
TablesView::TablesView(QWidget *parent) : QWidget(parent) {
    Theme::applyDarkPalette(this);

    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(12);

    // ---- Left: tabs containing the 4 tables --------------------------------
    auto *leftCol = new QVBoxLayout();
    tabs_ = new QTabWidget(this);
    tabs_->setDocumentMode(true);
    for (int t = 0; t < MAX_TABLES; t++) {
        views_[t]  = new QTableView(this);
        models_[t] = new SidTableModel(t, this);
        views_[t]->setModel(models_[t]);
        views_[t]->setItemDelegate(new TableCellDelegate(t, views_[t]));
        views_[t]->setShowGrid(false);
        views_[t]->verticalHeader()->hide();
        views_[t]->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        views_[t]->verticalHeader()->setDefaultSectionSize(20);
        views_[t]->setSelectionBehavior(QAbstractItemView::SelectRows);
        views_[t]->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(views_[t]->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex &, const QModelIndex &){ onCellSelectionChanged(); });
        tabs_->addTab(views_[t], tableName[t]);
        // Colour each tab title
        tabs_->tabBar()->setTabTextColor(t, tableTint[t]);
    }
    connect(tabs_, &QTabWidget::currentChanged, this, &TablesView::onTabChanged);
    leftCol->addWidget(tabs_, 1);

    // Action button row beneath table
    auto *btnRow = new QHBoxLayout();
    auto addBtn = [&](const QString &label, const QString &tip, void (TablesView::*slot)()) {
        auto *b = new QPushButton(label, this);
        b->setToolTip(tip);
        b->setMinimumWidth(64);
        connect(b, &QPushButton::clicked, this, slot);
        btnRow->addWidget(b);
        return b;
    };
    addBtn("Insert",  "Insert empty row above cursor (INS)",      &TablesView::insertRow);
    addBtn("Delete",  "Delete row at cursor (DEL)",               &TablesView::deleteRow);
    addBtn("Clear",   "Clear current cell to zero",               &TablesView::clearCell);
    addBtn("Negate",  "Negate speed parameter / relative note (Shift+N)", &TablesView::negate);
    addBtn("Limit→Time","Convert limit-based modulation to time (Shift+L)", &TablesView::limitToTime);
    addBtn("Optimize","Remove unused entries (Shift+O)",          &TablesView::optimize);
    addBtn("FF jump", "Set row to FF jump (RST equivalent)",      &TablesView::insertJump);
    btnRow->addStretch();
    leftCol->addLayout(btnRow);
    root->addLayout(leftCol, 3);

    // ---- Right: legend + cursor decoder + used-by --------------------------
    auto *rightCol = new QVBoxLayout();

    legend_ = new QLabel(this);
    legend_->setTextFormat(Qt::RichText);
    legend_->setWordWrap(true);
    legend_->setMinimumHeight(120);
    legend_->setAutoFillBackground(true);
    legend_->setContentsMargins(10, 8, 10, 8);
    QPalette lp = legend_->palette();
    lp.setColor(QPalette::Window, Theme::C::bgAlt);
    legend_->setPalette(lp);
    rightCol->addWidget(legend_);

    auto *decodeBox = new QGroupBox("Cursor decode", this);
    auto *dLay = new QVBoxLayout(decodeBox);
    decode_ = new QLabel(decodeBox);
    decode_->setWordWrap(true);
    decode_->setMinimumHeight(60);
    decode_->setFont(Theme::monoFont(11));
    QPalette dp = decode_->palette();
    dp.setColor(QPalette::WindowText, Theme::C::highlight);
    decode_->setPalette(dp);
    dLay->addWidget(decode_);
    rightCol->addWidget(decodeBox);

    auto *usedBox = new QGroupBox("Pointers to this row", this);
    auto *uLay = new QVBoxLayout(usedBox);
    usedBy_ = new QLabel(usedBox);
    usedBy_->setWordWrap(true);
    usedBy_->setFont(Theme::monoFont(10));
    QPalette up = usedBy_->palette();
    up.setColor(QPalette::WindowText, Theme::C::text);
    usedBy_->setPalette(up);
    uLay->addWidget(usedBy_);
    rightCol->addWidget(usedBox);

    rightCol->addStretch();
    root->addLayout(rightCol, 2);

    refresh();
}

void TablesView::onTabChanged(int idx) {
    if (updating_ || idx < 0 || idx >= MAX_TABLES) return;
    etnum = idx;
    onCellSelectionChanged();
    emit edited();
}

void TablesView::onCellSelectionChanged() {
    auto *v = views_[etnum];
    if (!v) return;
    auto idx = v->currentIndex();
    if (idx.isValid()) {
        etpos = idx.row();
        etcolumn = idx.column() >= 1 ? idx.column() - 1 : 0;
    }
    updateDecoder();
    updateUsedBy();
    updateLegend();
}

static QString tableLegendHtml(int t) {
    switch (t) {
        case 0: return QString(
            "<b style='color:#A1C181'>Wave table</b><br>"
            "<span style='color:#5A6470'>L bytes:</span><br>"
            "<b>$00</b> no change<br>"
            "<b>$01-$0F</b> delay 1..15 frames<br>"
            "<b>$10-$DF</b> waveform value (gate / saw / tri / pulse / noise bits)<br>"
            "<b>$E0-$EF</b> inaudible waveform $00-$0F<br>"
            "<b>$F0-$FE</b> execute pattern command 0XY-EXY (R = param)<br>"
            "<b>$FF</b> jump (R = target step, $00 = stop)<br>"
            "<span style='color:#5A6470'>R bytes:</span><br>"
            "<b>$00-$5F</b> relative note +0..+95<br>"
            "<b>$60-$7F</b> negative relative<br>"
            "<b>$80</b> hold frequency<br>"
            "<b>$81-$DF</b> absolute notes C#0..B-7"
        );
        case 1: return QString(
            "<b style='color:#6FA6CE'>Pulse table</b><br>"
            "<b>$01-$7F</b> modulate L ticks at signed-byte speed R<br>"
            "<b>$8X-$FX</b> set pulse width X|R (12-bit value $000-$FFF)<br>"
            "<b>$FF</b> jump (R = target, $00 = stop)"
        );
        case 2: return QString(
            "<b style='color:#D89B8C'>Filter table</b><br>"
            "<b>$00</b> set cutoff to R<br>"
            "<b>$01-$7F</b> modulate L ticks at signed-byte speed R<br>"
            "<b>$80-$F0</b> set filter params: hi-nibble = passband "
            "(90=LP, A0=BP, C0=HP, etc), R = resonance | channel-mask<br>"
            "<b>$FF</b> jump (R = target, $00 = stop)"
        );
        case 3: return QString(
            "<b style='color:#E0C16E'>Speed table</b><br>"
            "Shared by vibrato, portamento, funktempo. No jump opcode.<br>"
            "<b>Vibrato</b> L=direction-change speed, R=depth per tick<br>"
            "<b>Portamento</b> LR = signed 16-bit pitch step / tick<br>"
            "<b>Funktempo</b> L,R = two tempo values alternated per row<br>"
            "<b>L bit $80</b> enables note-independent vib depth / porta speed; "
            "R then specifies the divisor."
        );
    }
    return {};
}

void TablesView::updateLegend() {
    legend_->setText(tableLegendHtml(etnum));
}

void TablesView::updateDecoder() {
    unsigned char L = ltable[etnum][etpos];
    unsigned char R = rtable[etnum][etpos];
    decode_->setText(QString("%1 step $%2\nL=$%3  R=$%4\n→ %5")
        .arg(tableName[etnum])
        .arg(etpos + 1, 2, 16, QLatin1Char('0'))
        .arg(L, 2, 16, QLatin1Char('0'))
        .arg(R, 2, 16, QLatin1Char('0'))
        .arg(decodeCell(etnum, L, R))
        .toUpper());
}

void TablesView::updateUsedBy() {
    QStringList who;
    int row = etpos + 1; // table-pointer values are 1-based
    for (int i = 1; i < MAX_INSTR; i++) {
        if (instr[i].ptr[etnum] == row)
            who << QString("ins $%1").arg(i, 2, 16, QLatin1Char('0')).toUpper();
    }
    if (who.isEmpty()) usedBy_->setText("(no instruments point here)");
    else usedBy_->setText(QString("Used by: %1").arg(who.join(", ")));
}

void TablesView::refresh() {
    if (updating_) return;
    updating_ = true;
    for (int t = 0; t < MAX_TABLES; t++) models_[t]->refresh();
    if (tabs_->currentIndex() != etnum) tabs_->setCurrentIndex(etnum);
    auto *v = views_[etnum];
    if (v) v->setCurrentIndex(models_[etnum]->index(etpos, 1));
    updateDecoder();
    updateUsedBy();
    updateLegend();
    updating_ = false;
}

// ---- Actions hook into tablecommands() by setting key/rawkey/shiftpressed --
static void runTableCommand(int rawkeyVal, int asciiKey, bool shift) {
    rawkey = rawkeyVal;
    key = asciiKey;
    shiftpressed = shift ? 1 : 0;
    tablecommands();
    rawkey = 0;
    key = 0;
    shiftpressed = 0;
}

#include <SDL/SDL_keysym.h>

void TablesView::insertRow() {
    runTableCommand(SDLK_INSERT, 0, false);
    refresh();
    emit edited();
}
void TablesView::deleteRow() {
    runTableCommand(SDLK_DELETE, 0, false);
    refresh();
    emit edited();
}
void TablesView::negate() {
    runTableCommand('n', 'n', true);
    refresh();
    emit edited();
}
void TablesView::optimize() {
    runTableCommand('o', 'o', true);
    refresh();
    emit edited();
}
void TablesView::limitToTime() {
    runTableCommand('l', 'l', true);
    refresh();
    emit edited();
}
void TablesView::clearCell() {
    ltable[etnum][etpos] = 0;
    rtable[etnum][etpos] = 0;
    refresh();
    emit edited();
}
void TablesView::insertJump() {
    ltable[etnum][etpos] = 0xFF;
    rtable[etnum][etpos] = 0;
    refresh();
    emit edited();
}
