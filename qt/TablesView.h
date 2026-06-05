#pragma once
#include <QWidget>

class TablesView : public QWidget {
    Q_OBJECT
public:
    explicit TablesView(QWidget *parent = nullptr);
    void refresh() { update(); }

signals:
    void edited();

protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    int rowHeight = 16;
    int colWidth = 12;
};
