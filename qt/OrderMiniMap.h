#pragma once
#include <QWidget>

class OrderMiniMap : public QWidget {
    Q_OBJECT
public:
    explicit OrderMiniMap(QWidget *parent = nullptr);
    void refresh() { update(); }

signals:
    void positionChanged();

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
};
