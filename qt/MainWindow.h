#pragma once
#include <QMainWindow>

class PatternView;
class OrderView;
class InstrumentView;
class TablesView;
class SongNameView;
class OrderMiniMap;
class InstrumentQuickList;
class StatusStrip;
class QStackedWidget;
class QLabel;
class QTimer;
class QDockWidget;

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
    void toggleNtsc();
    void cycleMultiplier();
    void toggleFollowPlay();
    void cycleEditMode(bool backwards = false);
    void tick();

private:
    PatternView *pattern_ = nullptr;
    OrderView *order_ = nullptr;
    InstrumentView *instrument_ = nullptr;
    TablesView *tables_ = nullptr;
    SongNameView *songName_ = nullptr;
    QStackedWidget *stack_ = nullptr;
    OrderMiniMap *orderMap_ = nullptr;
    InstrumentQuickList *insQuick_ = nullptr;
    QDockWidget *orderMapDock_ = nullptr;
    QDockWidget *insQuickDock_ = nullptr;
    StatusStrip *statusStrip_ = nullptr;
    QTimer *timer_ = nullptr;

    void buildUi();
    void refreshAll();
    void syncStack();
};
