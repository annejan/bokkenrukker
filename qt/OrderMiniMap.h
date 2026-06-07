#pragma once
#include <QWidget>

class OrderMiniMap : public QWidget {
    Q_OBJECT
public:
    explicit OrderMiniMap(QWidget *parent = nullptr);
    void refresh() { update(); }
    QSize sizeHint() const override { return QSize(160, 400); }

    // Click-mode toggle wired from the dock header. When true, a plain click
    // moves all channels to the same orderlist row + ctrl-click moves only
    // the clicked one (legacy behaviour). When false, the two are swapped —
    // user works channel-by-channel by default.
    void setSelectAllChannels(bool b) { selectAll_ = b; }
    bool selectAllChannels() const     { return selectAll_; }

signals:
    void positionChanged();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;

private:
    bool selectAll_ = true;
};
