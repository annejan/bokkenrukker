#include "MainWindow.h"
#include "PatternView.h"
#include "OrderView.h"
#include "InstrumentView.h"
#include "TablesView.h"
#include "SongNameView.h"
#include "OrderMiniMap.h"
#include "InstrumentQuickList.h"
#include "StatusStrip.h"
#include "UndoStack.h"
#include <QUndoStack>

#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QStackedWidget>
#include <QLabel>
#include <QTimer>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QCloseEvent>
#include <QStatusBar>
#include <QWidget>
#include <QVBoxLayout>
#include <QToolButton>
#include <QStyle>
#include <QInputDialog>
#include <QActionGroup>
#include "Theme.h"
#include <cstring>

extern "C" {
#include "goattrk2.h"
#include "gcommon.h"
#include "gfile.h"
#include "gsong.h"
#include "gplay.h"
#include "gorder.h"

extern char songfilename[];
extern char songpath[];
extern char instrfilename[];
extern char instrpath[];
extern char songname[MAX_STR];
extern char authorname[MAX_STR];
extern char copyrightname[MAX_STR];
extern int esnum;
extern int editmode;
extern int followplay;
extern int einum;
extern int epchn;
extern unsigned sidmodel;
extern unsigned multiplier;
extern unsigned keypreset;
extern unsigned b, mr, writer, hardsid, ntsc, catweasel, interpolate, customclockrate;
// Microtonal globals (mirror src/goattrk2.c definitions).
extern float basepitch;
extern float equaldivisionsperoctave;
extern int tuningcount;
extern double tuning[96];
extern char specialnotenames[186];
extern char scalatuningfilepath[];
extern char tuningname[64];
int sound_init(unsigned b, unsigned mr, unsigned writer, unsigned hardsid,
               unsigned m, unsigned ntsc, unsigned multiplier,
               unsigned catweasel, unsigned interpolate, unsigned customclockrate);
int savesong(void);
int saveinstrument(void);
void loadinstrument(void);
void prevmultiplier(void);
void nextmultiplier(void);
void calculatefreqtable(void);
void setspecialnotenames(void);
void readscalatuningfile(void);
void resetnotenames(void);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    undoStack_ = new QUndoStack(this);
    undoStack_->setUndoLimit(64);
    buildUi();
    QSettings s("goattracker2-qt", "goattracker2-qt");
    QByteArray sp = s.value("songpath").toString().toLocal8Bit();
    QByteArray ip = s.value("instrpath").toString().toLocal8Bit();
    if (!sp.isEmpty()) std::strncpy(songpath, sp.constData(), MAX_PATHNAME - 1);
    if (!ip.isEmpty()) std::strncpy(instrpath, ip.constData(), MAX_PATHNAME - 1);
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::tick);
    timer_->start(20);
}

MainWindow::~MainWindow() {
    QSettings s("goattracker2-qt", "goattracker2-qt");
    s.setValue("songpath",  QString::fromLocal8Bit(songpath));
    s.setValue("instrpath", QString::fromLocal8Bit(instrpath));
}

void MainWindow::buildUi() {
    setWindowTitle("GoatTracker Qt");
    resize(1280, 800);

    // Central stacked editor with custom bottom status strip
    auto *centralWrap = new QWidget(this);
    auto *centralLay = new QVBoxLayout(centralWrap);
    centralLay->setContentsMargins(0, 0, 0, 0);
    centralLay->setSpacing(0);

    stack_ = new QStackedWidget(centralWrap);
    pattern_    = new PatternView(stack_);
    order_      = new OrderView(stack_);
    instrument_ = new InstrumentView(stack_);
    tables_     = new TablesView(stack_);
    songName_   = new SongNameView(stack_);
    stack_->insertWidget(EDIT_PATTERN, pattern_);
    stack_->insertWidget(EDIT_ORDERLIST, order_);
    stack_->insertWidget(EDIT_INSTRUMENT, instrument_);
    stack_->insertWidget(EDIT_TABLES, tables_);
    stack_->insertWidget(EDIT_NAMES, songName_);
    centralLay->addWidget(stack_, 1);

    statusStrip_ = new StatusStrip(centralWrap);
    centralLay->addWidget(statusStrip_);
    connect(statusStrip_, &StatusStrip::sidClicked, this, &MainWindow::toggleSidModel);
    connect(statusStrip_, &StatusStrip::followClicked, this, &MainWindow::toggleFollowPlay);
    connect(statusStrip_, &StatusStrip::ntscClicked, this, &MainWindow::toggleNtsc);
    connect(statusStrip_, &StatusStrip::tempoClicked, this, &MainWindow::cycleMultiplier);

    setCentralWidget(centralWrap);

    connect(pattern_, &PatternView::patternEdited, this, &MainWindow::refreshAll);
    connect(order_, &OrderView::edited, this, &MainWindow::refreshAll);
    connect(instrument_, &InstrumentView::edited, this, &MainWindow::refreshAll);
    connect(tables_, &TablesView::edited, this, &MainWindow::refreshAll);
    connect(songName_, &SongNameView::edited, this, &MainWindow::refreshAll);

    // Dock widgets
    orderMapDock_ = new QDockWidget("Order map", this);
    orderMap_ = new OrderMiniMap(orderMapDock_);
    orderMapDock_->setWidget(orderMap_);
    addDockWidget(Qt::LeftDockWidgetArea, orderMapDock_);
    connect(orderMap_, &OrderMiniMap::positionChanged, this, &MainWindow::refreshAll);

    insQuickDock_ = new QDockWidget("Instruments", this);
    insQuick_ = new InstrumentQuickList(insQuickDock_);
    insQuickDock_->setWidget(insQuick_);
    addDockWidget(Qt::RightDockWidgetArea, insQuickDock_);
    connect(insQuick_, &InstrumentQuickList::instrumentChosen, this, &MainWindow::refreshAll);

    // ---- Menus -----------------------------------------------------------
    auto *fileMenu = menuBar()->addMenu("&File");
    auto *openA = fileMenu->addAction("&Open .sng…");
    openA->setShortcut(Qt::CTRL | Qt::Key_O);
    connect(openA, &QAction::triggered, this, &MainWindow::openSong);
    auto *mergeA = fileMenu->addAction("&Merge .sng…");
    mergeA->setShortcut(Qt::CTRL | Qt::Key_M);
    connect(mergeA, &QAction::triggered, this, &MainWindow::mergeSong);
    auto *saveA = fileMenu->addAction("&Save");
    saveA->setShortcut(Qt::CTRL | Qt::Key_S);
    connect(saveA, &QAction::triggered, this, &MainWindow::saveSong);
    auto *saveAsA = fileMenu->addAction("Save &As…");
    saveAsA->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    connect(saveAsA, &QAction::triggered, this, &MainWindow::saveSongAs);
    fileMenu->addSeparator();
    auto *loadInsA = fileMenu->addAction("Load &Instrument…");
    connect(loadInsA, &QAction::triggered, this, &MainWindow::loadInstrument);
    auto *saveInsA = fileMenu->addAction("Save I&nstrument…");
    connect(saveInsA, &QAction::triggered, this, &MainWindow::saveInstrument);
    fileMenu->addSeparator();
    auto *quitA = fileMenu->addAction("&Quit");
    quitA->setShortcut(Qt::CTRL | Qt::Key_Q);
    connect(quitA, &QAction::triggered, this, &QMainWindow::close);

    // Edit menu — undo / redo
    auto *editMenu = menuBar()->addMenu("&Edit");
    auto *undoA = editMenu->addAction("&Undo");
    undoA->setShortcut(QKeySequence::Undo);
    connect(undoA, &QAction::triggered, this, &MainWindow::undo);
    auto *redoA = editMenu->addAction("&Redo");
    redoA->setShortcut(QKeySequence::Redo);
    connect(redoA, &QAction::triggered, this, &MainWindow::redo);

    auto *modeMenu = menuBar()->addMenu("&Mode");
    auto addMode = [&](const QString &label, int mode, Qt::Key shortcut) {
        auto *a = modeMenu->addAction(label);
        a->setShortcut(shortcut);
        connect(a, &QAction::triggered, this, [this, mode]{
            editmode = mode; syncStack(); refreshAll();
        });
    };
    addMode("&Pattern editor",   EDIT_PATTERN,    Qt::Key_F5);
    addMode("&Order/song editor", EDIT_ORDERLIST, Qt::Key_F6);
    addMode("&Instrument editor", EDIT_INSTRUMENT, Qt::Key_F7);
    addMode("&Tables editor",     EDIT_TABLES,    Qt::Key_F8);
    auto *namesA = modeMenu->addAction("Song&name editor");
    connect(namesA, &QAction::triggered, this, [this]{
        editmode = EDIT_NAMES; syncStack(); refreshAll();
    });

    auto *tabA = new QAction(this);
    tabA->setShortcut(Qt::Key_Tab);
    tabA->setShortcutContext(Qt::ApplicationShortcut);
    connect(tabA, &QAction::triggered, this, [this]{ cycleEditMode(false); });
    addAction(tabA);
    auto *backTabA = new QAction(this);
    backTabA->setShortcut(Qt::SHIFT | Qt::Key_Tab);
    backTabA->setShortcutContext(Qt::ApplicationShortcut);
    connect(backTabA, &QAction::triggered, this, [this]{ cycleEditMode(true); });
    addAction(backTabA);

    auto *viewMenu = menuBar()->addMenu("&View");
    viewMenu->addAction(orderMapDock_->toggleViewAction());
    viewMenu->addAction(insQuickDock_->toggleViewAction());
    auto *followA = viewMenu->addAction("Toggle &follow-play");
    followA->setShortcut(Qt::CTRL | Qt::Key_F);
    followA->setCheckable(true);
    connect(followA, &QAction::triggered, this, &MainWindow::toggleFollowPlay);

    // ---- Settings menu (microtonal / tuning / keypreset) ---------------
    auto *settingsMenu = menuBar()->addMenu("&Settings");

    auto *tuningMenu = settingsMenu->addMenu("&Tuning");
    auto *tuningGroup = new QActionGroup(this);
    tuningGroup->setExclusive(true);
    auto addTuning = [&](const QString &label, void (MainWindow::*slot)()) {
        auto *a = tuningMenu->addAction(label);
        a->setCheckable(true);
        tuningGroup->addAction(a);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };
    auto *t12A = addTuning("12-TET (default)", &MainWindow::setTuning12Tet);
    t12A->setChecked(true);
    addTuning("19-TET",            &MainWindow::setTuning19Tet);
    addTuning("24-TET",            &MainWindow::setTuning24Tet);
    addTuning("Custom N-TET…",     &MainWindow::setTuningCustomNTet);
    tuningMenu->addSeparator();
    auto *scalaA = tuningMenu->addAction("Load Scala .scl file…");
    connect(scalaA, &QAction::triggered, this, &MainWindow::loadScalaFile);
    tuningMenu->addSeparator();
    auto *resetTuningA = tuningMenu->addAction("&Reset to built-in table");
    connect(resetTuningA, &QAction::triggered, this, &MainWindow::resetTuning);

    auto *nameMenu = settingsMenu->addMenu("&Note names");
    auto *nameGroup = new QActionGroup(this);
    nameGroup->setExclusive(true);
    auto addNoteNames = [&](const QString &label, void (MainWindow::*slot)()) {
        auto *a = nameMenu->addAction(label);
        a->setCheckable(true);
        nameGroup->addAction(a);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };
    auto *n12A = addNoteNames("Standard 12 (C, C#, D…)", &MainWindow::setNoteNames12);
    n12A->setChecked(true);
    addNoteNames("Solfège (do, re, mi…)", &MainWindow::setNoteNamesSolfege);
    addNoteNames("Custom…",               &MainWindow::setNoteNamesCustom);
    nameMenu->addSeparator();
    auto *nResetA = nameMenu->addAction("Reset to defaults");
    connect(nResetA, &QAction::triggered, this, &MainWindow::setNoteNamesReset);

    auto *keyMenu = settingsMenu->addMenu("Note &entry layout");
    auto *keyGroup = new QActionGroup(this);
    keyGroup->setExclusive(true);
    auto addKey = [&](const QString &label, void (MainWindow::*slot)(), bool checked) {
        auto *a = keyMenu->addAction(label);
        a->setCheckable(true);
        a->setChecked(checked);
        keyGroup->addAction(a);
        connect(a, &QAction::triggered, this, slot);
        return a;
    };
    addKey("Protracker (default)", &MainWindow::setKeyPresetTracker, keypreset == KEY_TRACKER);
    addKey("DMC",                  &MainWindow::setKeyPresetDmc,     keypreset == KEY_DMC);
    addKey("Janko / isomorphic",   &MainWindow::setKeyPresetJanko,   keypreset == KEY_JANKO);

    auto *playMenu = menuBar()->addMenu("&Play");
    auto *playA = playMenu->addAction(QString::fromUtf8("⏮  Play from &beginning"));
    playA->setShortcut(Qt::Key_F1);
    playA->setToolTip("Play song from beginning (F1)");
    connect(playA, &QAction::triggered, this, &MainWindow::playFromBeginning);
    auto *playPosA = playMenu->addAction(QString::fromUtf8("▶  Play from &position / pause"));
    playPosA->setShortcut(Qt::Key_F2);
    playPosA->setToolTip("Toggle play/pause from current order position (F2). "
                         "Resumes near where you stopped.");
    connect(playPosA, &QAction::triggered, this, &MainWindow::playFromPos);
    playPosAction_ = playPosA;
    auto *playPatA = playMenu->addAction(QString::fromUtf8("⧈  Play one pa&ttern"));
    playPatA->setShortcut(Qt::Key_F3);
    playPatA->setToolTip("Loop the current pattern (F3)");
    connect(playPatA, &QAction::triggered, this, &MainWindow::playPattern);
    auto *stopA = playMenu->addAction(QString::fromUtf8("⏹  &Stop"));
    stopA->setShortcut(Qt::Key_F4);
    stopA->setToolTip("Stop playback (F4)");
    connect(stopA, &QAction::triggered, this, &MainWindow::stopSong);
    playMenu->addSeparator();
    auto *muteA = playMenu->addAction("&Mute current channel");
    muteA->setShortcut(Qt::SHIFT | Qt::Key_F4);
    connect(muteA, &QAction::triggered, this, &MainWindow::muteCurrentChannel);
    auto *decSpA = playMenu->addAction("&Decrease speed multiplier");
    decSpA->setShortcut(Qt::SHIFT | Qt::Key_F5);
    connect(decSpA, &QAction::triggered, this, &MainWindow::prevMultiplierSlot);
    auto *incSpA = playMenu->addAction("&Increase speed multiplier");
    incSpA->setShortcut(Qt::SHIFT | Qt::Key_F6);
    connect(incSpA, &QAction::triggered, this, &MainWindow::nextMultiplierSlot);
    auto *sidA = playMenu->addAction("Toggle SID &model (6581/8580)");
    sidA->setShortcut(Qt::SHIFT | Qt::Key_F8);
    connect(sidA, &QAction::triggered, this, &MainWindow::toggleSidModel);

    auto *tb = addToolBar("Transport");
    tb->setMovable(false);
    tb->setStyleSheet(QString(
        "QToolBar { background:%1; spacing:4px; padding:6px; border:0; }"
        "QToolButton { color:%2; background:%3; padding:5px 10px; border:1px solid %4; border-radius:4px; min-width:0; }"
        "QToolButton:hover { background:%5; }"
        // Play family — distinct shades + icon-sized
        "QToolButton#playBegin  { background:#2F8C3A; color:#FFFFFF; border-color:#3FB950; font-weight:bold; }"
        "QToolButton#playBegin:hover  { background:#3FB950; }"
        "QToolButton#playPos    { background:#1F5E7A; color:#E0E6EE; border-color:#3892B5; }"
        "QToolButton#playPos:hover    { background:#3892B5; }"
        "QToolButton#playPatt   { background:#7A5A1F; color:#E0E6EE; border-color:#D9A441; }"
        "QToolButton#playPatt:hover   { background:#B58E38; }"
        "QToolButton#stopBtn    { background:#8C2F2F; color:#FFFFFF; border-color:#E5484D; font-weight:bold; }"
        "QToolButton#stopBtn:hover    { background:#E5484D; }"
    )
        .arg(Theme::C::bgAlt.name())
        .arg(Theme::C::text.name())
        .arg(Theme::C::bgBase.name())
        .arg(Theme::C::sep.name())
        .arg(Theme::C::editRow.name()));

    // Transport group: distinct color per play variant + red stop
    tb->addAction(playA);
    tb->addAction(playPosA);
    tb->addAction(playPatA);
    tb->addAction(stopA);
    if (auto *btn = qobject_cast<QToolButton*>(tb->widgetForAction(playA)))
        { btn->setObjectName("playBegin"); btn->setText("⏮ Begin"); }
    if (auto *btn = qobject_cast<QToolButton*>(tb->widgetForAction(playPosA)))
        { btn->setObjectName("playPos");   btn->setText("▶ Pos"); }
    // We re-label the toolbar button between ▶ Pos and ⏸ Pause in tick().
    if (auto *btn = qobject_cast<QToolButton*>(tb->widgetForAction(playPatA)))
        { btn->setObjectName("playPatt");  btn->setText("⧈ Patt"); }
    if (auto *btn = qobject_cast<QToolButton*>(tb->widgetForAction(stopA)))
        { btn->setObjectName("stopBtn");   btn->setText("⏹ Stop"); }

    auto addSpacer = [&](int w = 28) {
        auto *spacer = new QWidget(tb);
        spacer->setFixedWidth(w);
        tb->addWidget(spacer);
    };

    addSpacer();

    // Mode group
    tb->addAction(modeMenu->actions().at(0));
    tb->addAction(modeMenu->actions().at(1));
    tb->addAction(modeMenu->actions().at(2));
    tb->addAction(modeMenu->actions().at(3));

    addSpacer();

    // Follow-play stays on the toolbar (frequently toggled); the two dock
    // toggles ("Order map" / "Instruments") live in the View menu only, to
    // avoid visual collision with the mode switches above.
    tb->addAction(followA);

    tb->style()->unpolish(tb);
    tb->style()->polish(tb);

    statusStrip_->showMessage("Ready. Ctrl+O to open a song.");
    syncStack();
}

void MainWindow::syncStack() {
    if (editmode < 0 || editmode > EDIT_NAMES) editmode = EDIT_PATTERN;
    stack_->setCurrentIndex(editmode);
    QWidget *w = stack_->currentWidget();
    if (w) w->setFocus();
}

void MainWindow::cycleEditMode(bool backwards) {
    if (backwards) editmode--;
    else editmode++;
    if (editmode > EDIT_NAMES) editmode = EDIT_PATTERN;
    if (editmode < EDIT_PATTERN) editmode = EDIT_NAMES;
    syncStack();
    refreshAll();
}

static void rememberDir(const QString &filePath, char *pathSlot, int slotSize) {
    QFileInfo fi(filePath);
    QString dir = fi.absolutePath();
    if (!dir.endsWith('/')) dir += '/';
    QByteArray ba = dir.toLocal8Bit();
    std::strncpy(pathSlot, ba.constData(), slotSize - 1);
    pathSlot[slotSize - 1] = 0;
}

void MainWindow::loadSongFile(const QString &path) {
    QByteArray ba = path.toLocal8Bit();
    std::strncpy(songfilename, ba.constData(), MAX_FILENAME - 1);
    songfilename[MAX_FILENAME - 1] = 0;
    rememberDir(path, songpath, MAX_PATHNAME);
    loadsong();
    countpatternlengths();
    refreshAll();
    statusStrip_->showMessage(QString("Loaded: %1").arg(path));
}

void MainWindow::openSong() {
    QString start = songpath[0] ? QString::fromLocal8Bit(songpath)
                                : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getOpenFileName(this, "Open Song", start, "SNG (*.sng);;All (*.*)");
    if (fn.isEmpty()) return;
    loadSongFile(fn);
}

// Append a second song's patterns / orderlists / instruments / tables onto
// the currently-loaded song. Uses the v2.73 mergesong() C helper which
// reads from the global `songfilename` slot.
void MainWindow::mergeSong() {
    QString start = songpath[0] ? QString::fromLocal8Bit(songpath)
                                : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getOpenFileName(this, "Merge Song", start, "SNG (*.sng);;All (*.*)");
    if (fn.isEmpty()) return;
    QByteArray before = beginEdit();
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(songfilename, ba.constData(), MAX_FILENAME - 1);
    songfilename[MAX_FILENAME - 1] = 0;
    rememberDir(fn, songpath, MAX_PATHNAME);
    mergesong();
    countpatternlengths();
    refreshAll();
    endEdit(before, "Merge song");
    statusStrip_->showMessage(QString("Merged: %1").arg(fn));
}

void MainWindow::saveSong() {
    if (!songfilename[0]) { saveSongAs(); return; }
    if (savesong()) statusStrip_->showMessage(QString("Saved: %1").arg(songfilename));
    else statusStrip_->showMessage("Save failed");
}

void MainWindow::saveSongAs() {
    QString start = songpath[0] ? QString::fromLocal8Bit(songpath)
                  : songfilename[0] ? QString::fromLocal8Bit(songfilename)
                  : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getSaveFileName(this, "Save Song", start, "SNG (*.sng)");
    if (fn.isEmpty()) return;
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(songfilename, ba.constData(), MAX_FILENAME - 1);
    songfilename[MAX_FILENAME - 1] = 0;
    rememberDir(fn, songpath, MAX_PATHNAME);
    saveSong();
}

void MainWindow::loadInstrument() {
    QString start = instrpath[0] ? QString::fromLocal8Bit(instrpath)
                                 : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getOpenFileName(this, "Load Instrument", start, "INS (*.ins);;All (*.*)");
    if (fn.isEmpty()) return;
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(instrfilename, ba.constData(), MAX_FILENAME - 1);
    instrfilename[MAX_FILENAME - 1] = 0;
    rememberDir(fn, instrpath, MAX_PATHNAME);
    loadinstrument();
    refreshAll();
    statusStrip_->showMessage(QString("Loaded ins: %1").arg(fn));
}

void MainWindow::saveInstrument() {
    QString start = instrpath[0] ? QString::fromLocal8Bit(instrpath)
                  : instrfilename[0] ? QString::fromLocal8Bit(instrfilename)
                  : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getSaveFileName(this, "Save Instrument", start, "INS (*.ins)");
    if (fn.isEmpty()) return;
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(instrfilename, ba.constData(), MAX_FILENAME - 1);
    instrfilename[MAX_FILENAME - 1] = 0;
    rememberDir(fn, instrpath, MAX_PATHNAME);
    if (saveinstrument()) statusStrip_->showMessage(QString("Saved ins: %1").arg(fn));
    else statusStrip_->showMessage("Save instrument failed");
}

void MainWindow::refreshAll() {
    pattern_->refresh();
    order_->refresh();
    instrument_->refresh();
    tables_->refresh();
    songName_->refresh();
    orderMap_->refresh();
    insQuick_->refresh();
    statusStrip_->refresh();
}

void MainWindow::playFromBeginning() { followplay = 1; initsong(esnum, PLAY_BEGINNING); }
void MainWindow::playFromPos() {
    // Toggle: stop if currently playing, otherwise resume from order cursor.
    if (isplaying()) {
        stopsong();
        statusStrip_->showMessage("Paused");
    } else {
        followplay = 1;
        initsong(esnum, PLAY_POS);
    }
}
void MainWindow::playPattern()       { followplay = 1; initsong(esnum, PLAY_PATTERN); }
void MainWindow::stopSong()          { stopsong(); }

void MainWindow::muteCurrentChannel() {
    mutechannel(epchn);
}

void MainWindow::prevMultiplierSlot() { prevmultiplier(); refreshAll(); }
void MainWindow::nextMultiplierSlot() { nextmultiplier(); refreshAll(); }
void MainWindow::toggleSidModel() {
    sidmodel ^= 1;
    sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier,
               catweasel, interpolate, customclockrate);
    statusStrip_->showMessage(sidmodel ? "Switched to 8580 SID"
                                       : "Switched to 6581 SID");
    refreshAll();
}

void MainWindow::toggleNtsc() {
    ntsc ^= 1;
    sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier,
               catweasel, interpolate, customclockrate);
    statusStrip_->showMessage(ntsc ? "Switched to NTSC 60Hz"
                                   : "Switched to PAL 50Hz");
    refreshAll();
}

void MainWindow::cycleMultiplier() {
    // ½x (0) → 1 → 2 → 3 → 4 → ½x
    if (multiplier == 0)      multiplier = 1;
    else if (multiplier < 4)  multiplier++;
    else                       multiplier = 0;
    sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier,
               catweasel, interpolate, customclockrate);
    statusStrip_->showMessage(QString("Speed multiplier: %1")
        .arg(multiplier == 0 ? "½x" : QString("%1x").arg(multiplier)));
    refreshAll();
}
void MainWindow::toggleFollowPlay() {
    followplay = !followplay;
    statusStrip_->showMessage(followplay ? "Follow-play ON" : "Follow-play OFF");
    refreshAll();
}

void MainWindow::tick() {
    pattern_->tickScope();
    if (stack_->currentIndex() == EDIT_PATTERN) pattern_->refresh();
    if (isplaying()) orderMap_->refresh();
    statusStrip_->refresh();
    // Re-label the Pos toolbar button between ▶ Pos and ⏸ Pause as transport
    // state changes, so the button always shows the action it will perform.
    if (playPosAction_) {
        const QList<QWidget*> widgets = playPosAction_->associatedWidgets();
        for (QWidget *w : widgets) {
            if (auto *btn = qobject_cast<QToolButton*>(w)) {
                const QString desired = isplaying() ? "⏸ Pause" : "▶ Pos";
                if (btn->text() != desired) btn->setText(desired);
            }
        }
    }
}

QByteArray MainWindow::beginEdit() {
    return captureSongSnapshot();
}

void MainWindow::endEdit(QByteArray before, const QString &label) {
    auto *cmd = new SongSnapshotCommand(std::move(before), label);
    undoStack_->push(cmd);
}

void MainWindow::undo() {
    if (!undoStack_->canUndo()) {
        statusStrip_->showMessage("Nothing to undo");
        return;
    }
    QString label = undoStack_->undoText();
    undoStack_->undo();
    statusStrip_->showMessage(QString("Undo: %1").arg(label));
    refreshAll();
}

void MainWindow::redo() {
    if (!undoStack_->canRedo()) {
        statusStrip_->showMessage("Nothing to redo");
        return;
    }
    QString label = undoStack_->redoText();
    undoStack_->redo();
    statusStrip_->showMessage(QString("Redo: %1").arg(label));
    refreshAll();
}

// ----- Microtonal / Scala / keyboard layout settings ------------------------
// Backport of v2.75's -Q / -J / -Y / Janko features. The shared C
// functions (calculatefreqtable / setspecialnotenames / readscalatuningfile)
// live in qt_globals.c. Any slot that changes the freq table also kicks
// notename[] back to defaults first so we don't reuse stale microtonal
// labels from a previous Scala load.

static void applyNTet(float n, const char *label, StatusStrip *strip) {
    equaldivisionsperoctave = n;
    tuningcount = 0;            // disables the ratio path in calculatefreqtable()
    if (basepitch <= 0.0f) basepitch = 440.0f;
    calculatefreqtable();
    resetnotenames();
    if (strip) strip->showMessage(QString("Tuning: %1").arg(label));
}

void MainWindow::setTuning12Tet() { applyNTet(12.0f, "12-TET", statusStrip_); }
void MainWindow::setTuning19Tet() { applyNTet(19.0f, "19-TET", statusStrip_); }
void MainWindow::setTuning24Tet() { applyNTet(24.0f, "24-TET", statusStrip_); }

void MainWindow::setTuningCustomNTet() {
    bool ok = false;
    double n = QInputDialog::getDouble(this, "Custom N-TET",
        "Equal divisions per octave\n(e.g. 31, or 8.2019143 for Bohlen-Pierce):",
        equaldivisionsperoctave, 1.0, 96.0, 4, &ok);
    if (!ok) return;
    applyNTet((float)n, QString("%1-TET").arg(n).toUtf8().constData(), statusStrip_);
}

void MainWindow::loadScalaFile() {
    QString fn = QFileDialog::getOpenFileName(this, "Load Scala tuning",
        QString(), "Scala tuning (*.scl);;All (*.*)");
    if (fn.isEmpty()) return;
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(scalatuningfilepath, ba.constData(), MAX_PATHNAME - 1);
    scalatuningfilepath[MAX_PATHNAME - 1] = 0;
    tuningcount = 0;
    specialnotenames[0] = '\0';
    readscalatuningfile();
    if (tuningcount <= 0) {
        statusStrip_->showMessage("Scala load failed (no tuning ratios parsed)");
        return;
    }
    if (basepitch <= 0.0f) basepitch = 440.0f;
    calculatefreqtable();
    resetnotenames();
    if (specialnotenames[0] && specialnotenames[1]) setspecialnotenames();
    statusStrip_->showMessage(QString("Loaded Scala: %1 (%2 ratios)")
        .arg(tuningname[0] ? QString::fromLocal8Bit(tuningname) : QFileInfo(fn).fileName())
        .arg(tuningcount));
    refreshAll();
}

void MainWindow::resetTuning() {
    equaldivisionsperoctave = 12.0f;
    tuningcount = 0;
    specialnotenames[0] = '\0';
    scalatuningfilepath[0] = '\0';
    basepitch = 0.0f;  // signal "use built-in baked freqtable"
    // We can't restore the original baked freqtable without re-reading
    // gplay.c's initialiser, so just recompute 12-TET at 440Hz. Close enough
    // for hearing the reset land; matches what -G440 -Q12 produces.
    basepitch = 440.0f;
    calculatefreqtable();
    basepitch = 0.0f;
    resetnotenames();
    statusStrip_->showMessage("Tuning reset to 12-TET defaults");
    refreshAll();
}

void MainWindow::setNoteNames12() {
    resetnotenames();
    specialnotenames[0] = '\0';
    statusStrip_->showMessage("Note names: standard 12");
    refreshAll();
}

void MainWindow::setNoteNamesSolfege() {
    // Two-char-per-note pack for solfège — sharps keep the C# style label
    // so the cycle still has 12 entries (matches 12-TET).
    static const char *names12[12] = {
        "Do","C#","Re","D#","Mi","Fa","F#","So","G#","La","A#","Ti"
    };
    int i = 0;
    for (int n = 0; n < 12; n++) {
        specialnotenames[i++] = names12[n][0];
        specialnotenames[i++] = names12[n][1];
    }
    specialnotenames[i] = '\0';
    setspecialnotenames();
    statusStrip_->showMessage("Note names: solfège");
    refreshAll();
}

void MainWindow::setNoteNamesCustom() {
    bool ok = false;
    QString cur = QString::fromLocal8Bit(specialnotenames);
    QString s = QInputDialog::getText(this, "Custom note names",
        "Two chars per note within an octave/cycle.\n"
        "E.g. C-DbD-EbE-F-GbG-AbA-BbB-",
        QLineEdit::Normal, cur, &ok);
    if (!ok) return;
    QByteArray ba = s.toLocal8Bit();
    if (ba.size() < 2) {
        statusStrip_->showMessage("Need at least one 2-char name");
        return;
    }
    std::strncpy(specialnotenames, ba.constData(), sizeof(specialnotenames) - 1);
    specialnotenames[sizeof(specialnotenames) - 1] = '\0';
    setspecialnotenames();
    statusStrip_->showMessage("Note names: custom");
    refreshAll();
}

void MainWindow::setNoteNamesReset() {
    specialnotenames[0] = '\0';
    resetnotenames();
    statusStrip_->showMessage("Note names reset");
    refreshAll();
}

void MainWindow::setKeyPresetTracker() {
    keypreset = KEY_TRACKER;
    statusStrip_->showMessage("Note entry: Protracker layout");
}
void MainWindow::setKeyPresetDmc() {
    keypreset = KEY_DMC;
    statusStrip_->showMessage("Note entry: DMC layout");
}
void MainWindow::setKeyPresetJanko() {
    keypreset = KEY_JANKO;
    statusStrip_->showMessage("Note entry: Janko (isomorphic) layout");
}
