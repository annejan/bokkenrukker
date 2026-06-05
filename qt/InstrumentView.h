#pragma once
#include <QWidget>

class InstrumentView : public QWidget {
    Q_OBJECT
public:
    explicit InstrumentView(QWidget *parent = nullptr);
    void refresh() { update(); }

signals:
    void edited();

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    int rowHeight = 18;
    int colWidth = 12;
};
