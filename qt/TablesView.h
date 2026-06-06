#pragma once
#include <QWidget>
#include <QAbstractTableModel>

class QTabWidget;
class QTableView;
class QLabel;
class QPushButton;

class SidTableModel : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit SidTableModel(int tableIndex, QObject *parent = nullptr);
    int rowCount(const QModelIndex &p = {}) const override;
    int columnCount(const QModelIndex &p = {}) const override;
    QVariant data(const QModelIndex &i, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation o, int role) const override;
    bool setData(const QModelIndex &i, const QVariant &v, int role) override;
    Qt::ItemFlags flags(const QModelIndex &i) const override;
    void refresh();
private:
    int t_;
};

class TablesView : public QWidget {
    Q_OBJECT
public:
    explicit TablesView(QWidget *parent = nullptr);
    void refresh();

signals:
    void edited();

private slots:
    void onTabChanged(int idx);
    void onCellSelectionChanged();
    void insertRow();
    void deleteRow();
    void negate();
    void optimize();
    void limitToTime();
    void clearCell();
    void insertJump();

private:
    QTabWidget *tabs_ = nullptr;
    QTableView *views_[4] = {nullptr,nullptr,nullptr,nullptr};
    SidTableModel *models_[4] = {nullptr,nullptr,nullptr,nullptr};
    QLabel *decode_ = nullptr;
    QLabel *legend_ = nullptr;
    QLabel *usedBy_ = nullptr;
    bool updating_ = false;

    void updateDecoder();
    void updateLegend();
    void updateUsedBy();
};
