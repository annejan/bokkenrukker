#pragma once
#include <QAbstractScrollArea>

class PatternView : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit PatternView(QWidget *parent = nullptr);

public slots:
    void refresh();

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
    void updateScrollRange();
};
