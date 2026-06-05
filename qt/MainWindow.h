#pragma once
#include <QMainWindow>

class PatternView;
class OrderView;
class InstrumentView;
class TablesView;
class SongNameView;
class QStackedWidget;
class QLabel;
class QTimer;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void loadSongFile(const QString &path);

private slots:
    void openSong();
    void saveSong();
    void saveSongAs();
    void loadInstrument();
    void saveInstrument();
    void playFromBeginning();
    void playFromPos();
    void playPattern();
    void stopSong();
    void muteCurrentChannel();
    void prevMultiplierSlot();
    void nextMultiplierSlot();
    void toggleSidModel();
    void cycleEditMode(bool backwards = false);
    void tick();

private:
    PatternView *pattern_ = nullptr;
    OrderView *order_ = nullptr;
    InstrumentView *instrument_ = nullptr;
    TablesView *tables_ = nullptr;
    SongNameView *songName_ = nullptr;
    QStackedWidget *stack_ = nullptr;
    QLabel *songInfo_ = nullptr;
    QLabel *statusOctave_ = nullptr;
    QLabel *statusInstr_ = nullptr;
    QLabel *statusMode_ = nullptr;
    QLabel *statusFollow_ = nullptr;
    QTimer *timer_ = nullptr;

    void buildUi();
    void refreshSongInfo();
    void refreshStatus();
    void refreshAll();
    void syncStack();
};
