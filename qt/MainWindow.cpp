#include "MainWindow.h"
#include "PatternView.h"
#include "OrderView.h"
#include "InstrumentView.h"
#include "TablesView.h"
#include "SongNameView.h"
#include "OrderMiniMap.h"
#include "InstrumentQuickList.h"
#include "StatusStrip.h"

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
extern unsigned b, mr, writer, hardsid, ntsc, catweasel, interpolate, customclockrate;
int sound_init(unsigned b, unsigned mr, unsigned writer, unsigned hardsid,
               unsigned m, unsigned ntsc, unsigned multiplier,
               unsigned catweasel, unsigned interpolate, unsigned customclockrate);
int savesong(void);
int saveinstrument(void);
void loadinstrument(void);
void prevmultiplier(void);
void nextmultiplier(void);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    buildUi();
    // Restore last-used directories so File dialogs reopen where the user left off.
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

    auto *playMenu = menuBar()->addMenu("&Play");
    auto *playA = playMenu->addAction(QString::fromUtf8("⏮  Play from &beginning"));
    playA->setShortcut(Qt::Key_F1);
    playA->setToolTip("Play song from beginning (F1)");
    connect(playA, &QAction::triggered, this, &MainWindow::playFromBeginning);
    auto *playPosA = playMenu->addAction(QString::fromUtf8("▶  Play from &position"));
    playPosA->setShortcut(Qt::Key_F2);
    playPosA->setToolTip("Play song from current order position (F2)");
    connect(playPosA, &QAction::triggered, this, &MainWindow::playFromPos);
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
void MainWindow::playFromPos()       { followplay = 1; initsong(esnum, PLAY_POS); }
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
}
