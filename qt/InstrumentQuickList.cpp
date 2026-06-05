#include "InstrumentQuickList.h"
#include "Theme.h"

#include <cstring>

extern "C" {
#include "gcommon.h"
extern INSTR instr[MAX_INSTR];
extern int einum;
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
