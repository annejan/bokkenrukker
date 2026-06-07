#pragma once
#include <QWidget>

class QSpinBox;
class QLineEdit;
class QComboBox;
class QLabel;
class AdsrPreview;
class WavetablePreview;

class InstrumentView : public QWidget {
    Q_OBJECT
public:
    explicit InstrumentView(QWidget *parent = nullptr);
    void refresh();

signals:
    void edited();

private slots:
    void onNameChanged(const QString &);
    void onAdChanged(int);
    void onSrChanged(int);
    void onWtChanged(int);
    void onPtChanged(int);
    void onFtChanged(int);
    void onVibParamChanged(int);
    void onVibDelayChanged(int);
    void onGateChanged(int);
    void onFirstWaveChanged(int);
    void onGotoWaveTable();
    void onGotoPulseTable();
    void onGotoFilterTable();
    void onTestNote();
    void onSilenceTestNote();
    void applyPreset(int index);

private:
    class QComboBox *presetBox_ = nullptr;
    QLineEdit *name_ = nullptr;
    QSpinBox *attack_, *decay_, *sustain_, *release_;
    QSpinBox *wave_, *pulse_, *filter_, *vibParam_;
    QSpinBox *vibDelay_, *gateTimer_, *firstWave_;
    AdsrPreview *adsr_;
    WavetablePreview *wavePrev_;
    QLabel *summary_;
    bool updating_ = false;

    void readFromGlobals();
    void writeAd();
    void writeSr();
};
