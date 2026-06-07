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
    void setColorsEnabled(bool on);
    bool colorsEnabled() const { return colorsOn_; }

signals:
    void instrumentChosen();

private slots:
    void onRowChanged(int row);
    void tickFlash();

private:
    bool updating_ = false;
    bool blinkEnabled_ = false;
    bool colorsOn_ = false;
    QTimer flashTimer_;
    // Decay counter per (instrument, channel). MAX_CHN=6 covers stereo.
    uint8_t flash_[256][6] = {{0}};
};
