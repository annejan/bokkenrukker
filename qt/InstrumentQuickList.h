#pragma once
#include <QListWidget>

class InstrumentQuickList : public QListWidget {
    Q_OBJECT
public:
    explicit InstrumentQuickList(QWidget *parent = nullptr);
    void refresh();

signals:
    void instrumentChosen();

private slots:
    void onRowChanged(int row);

private:
    bool updating_ = false;
};
