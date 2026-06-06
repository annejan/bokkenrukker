#pragma once
#include <QListWidget>
#include <QTimer>
#include <cstdint>

class InstrumentQuickList : public QListWidget {
    Q_OBJECT
public:
    explicit InstrumentQuickList(QWidget *parent = nullptr);
    void refresh();
    void setBlinkEnabled(bool on);
    bool blinkEnabled() const { return blinkEnabled_; }

signals:
    void instrumentChosen();

private slots:
    void onRowChanged(int row);
    void tickFlash();

private:
    bool updating_ = false;
    bool blinkEnabled_ = false;
    QTimer flashTimer_;
    uint8_t flash_[256] = {0};   // decay counter per instrument
};
