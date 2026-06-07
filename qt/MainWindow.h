#pragma once
#include <QMainWindow>
#include <QByteArray>

class PatternView;
class OrderView;
class InstrumentView;
class TablesView;
class SongNameView;
class OrderMiniMap;
class InstrumentQuickList;
class StatusStrip;
class QStackedWidget;
class QWidget;
class QLabel;
class QTimer;
class QDockWidget;
class QUndoStack;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void loadSongFile(const QString &path);

    // Editors call beginEdit() before mutating the song, then endEdit(label)
    // to push the resulting state onto the undo stack with the given label.
    QByteArray beginEdit();
    void endEdit(QByteArray before, const QString &label);

    // Exposed to the RPC layer so a test harness can drive time forward
    // deterministically instead of waiting on the 50 Hz QTimer.
    void tickOnce() { tick(); }
    QWidget *activeEditorWidget() const;

private slots:
    void openSong();
    void mergeSong();
    void saveSong();
    void saveSongAs();
    void packAndRelocate();
    void loadInstrument();
    void saveInstrument();
    void playFromBeginning();
    void playFromPos();   // toggles: stop if playing, resume from position if stopped
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

private slots:
    void undo();
    void redo();

    // Microtonal / tuning settings (backport from v2.75)
    void setTuning12Tet();
    void setTuning19Tet();
    void setTuning24Tet();
    void setTuningCustomNTet();
    void loadScalaFile();
    void resetTuning();
    void setNoteNames12();
    void setNoteNamesSolfege();
    void setNoteNamesCustom();
    void setNoteNamesReset();
    void setKeyPresetTracker();
    void setKeyPresetDmc();
    void setKeyPresetJanko();
    void toggleStereoMode(bool on);
    void toggleSid2Model();
    void cycleSid2();   // status-strip click: off -> 6581 -> 8580 -> off

private:
    QUndoStack *undoStack_ = nullptr;
    QAction *playPosAction_ = nullptr;
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
    QWidget     *patternBar_ = nullptr;       // toolbar above the stack —
    QLabel      *patternBarOct_ = nullptr;    // only visible on Pattern
    QLabel      *patternBarLen_ = nullptr;    // editor.
    QTimer *timer_ = nullptr;

    void buildUi();
    void refreshAll();
    void syncStack();

    void shrinkPattern();
    void growPattern();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
};
