#pragma once
#include <QFrame>

class QLabel;
class ClickableLabel;

class StatusStrip : public QFrame {
    Q_OBJECT
public:
    explicit StatusStrip(QWidget *parent = nullptr);
    void refresh();
    void showMessage(const QString &t, int ms = 3000);

signals:
    void sidClicked();
    void followClicked();

private:
    QLabel *transport_;
    QLabel *position_;
    QLabel *tempo_;
    QLabel *octave_;
    QLabel *instr_;
    ClickableLabel *sid_;
    ClickableLabel *follow_;
    QLabel *message_;
};
