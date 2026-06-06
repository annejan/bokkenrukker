#pragma once
#include <QAbstractScrollArea>
#include <array>
#include <deque>
extern "C" {
#include "gcommon.h"
}

class PatternView : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit PatternView(QWidget *parent = nullptr);

public slots:
    void refresh();
    void tickScope();

signals:
    void patternEdited();

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    bool event(QEvent *e) override;

private:
    int rowHeight = 16;
    int colWidth = 14;
    int visibleRows() const;
    int gridTopOffset() const;
    void updateScrollRange();

    // Per-channel envelope ring buffer (~60 frames). MAX_CHN comes from the
    // C side via gcommon.h — 3 for mono, 6 for the stereo build.
    static constexpr int kScopeLen = 64;
    std::array<std::array<unsigned char, kScopeLen>, MAX_CHN> scope_{};
    int scopeHead_ = 0;

    // Layout constants computed at construction
    int rowNumW_ = 0;
    int chnW_ = 0;
    int vuStripH_ = 0;
    int scopeStripH_ = 0;
    int headerStripH_ = 0;

    int channelAtX(int x) const;
};
