#pragma once
#include <QWidget>

class QLineEdit;
class QLabel;

class SongNameView : public QWidget {
    Q_OBJECT
public:
    explicit SongNameView(QWidget *parent = nullptr);
    void refresh();

signals:
    void edited();

private slots:
    void onTitleChanged(const QString &t);
    void onAuthorChanged(const QString &t);
    void onCopyrightChanged(const QString &t);

private:
    QLineEdit *title_;
    QLineEdit *author_;
    QLineEdit *copyright_;
};
