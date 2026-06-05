#pragma once
#include <QFrame>

class QLabel;

class StatusStrip : public QFrame {
    Q_OBJECT
public:
    explicit StatusStrip(QWidget *parent = nullptr);
    void refresh();
    void showMessage(const QString &t, int ms = 3000);

private:
    QLabel *transport_;
    QLabel *position_;
    QLabel *tempo_;
    QLabel *octave_;
    QLabel *instr_;
    QLabel *sid_;
    QLabel *follow_;
    QLabel *message_;
};
