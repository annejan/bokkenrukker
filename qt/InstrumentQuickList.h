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
    // Colours are opt-in per instrument now — the user double-clicks a row
    // (handled here) to flip the instrument's bit in the global mask. The
    // master button in the dock header still routes through setAllColored.
    void setAllColored(bool on);

signals:
    void instrumentChosen();
    void colorMaskChanged();  // dock header + pattern view repaint

private slots:
    void onRowChanged(int row);
    void tickFlash();

protected:
    void mouseDoubleClickEvent(QMouseEvent *e) override;

private:
    bool updating_ = false;
    bool blinkEnabled_ = false;
    QTimer flashTimer_;
    // Decay counter per (instrument, channel). MAX_CHN=6 covers stereo.
    uint8_t flash_[256][6] = {{0}};
};
