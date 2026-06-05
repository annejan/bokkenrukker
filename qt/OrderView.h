#pragma once
#include <QWidget>

class OrderView : public QWidget {
    Q_OBJECT
public:
    explicit OrderView(QWidget *parent = nullptr);
    void refresh() { update(); }

signals:
    void edited();

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    bool event(QEvent *e) override;

private:
    int rowHeight = 16;
    int colWidth = 12;
};
