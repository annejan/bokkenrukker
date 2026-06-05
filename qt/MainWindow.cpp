#include "MainWindow.h"
#include "PatternView.h"
#include "OrderView.h"
#include "InstrumentView.h"
#include "TablesView.h"
#include "SongNameView.h"

#include <QDockWidget>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QShortcut>
#include <QKeySequence>
#include <QStatusBar>
#include <QStackedWidget>
#include <QLabel>
#include <QTimer>
#include <QStandardPaths>
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
extern unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
extern int songlen[MAX_SONGS][MAX_CHN];
extern int esnum;
extern int editmode;
extern int followplay;
extern int epoctave;
extern int einum;
extern int epchn;
extern unsigned sidmodel;
extern unsigned multiplier;
extern unsigned adparam;
int savesong(void);
int saveinstrument(void);
void loadinstrument(void);
void calculatefreqtable(void);
void prevmultiplier(void);
void nextmultiplier(void);
void initchannels(void);
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    buildUi();
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &MainWindow::tick);
    timer_->start(20); // ~50 Hz
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
    setWindowTitle("GoatTracker Qt");
    resize(1200, 760);

    stack_ = new QStackedWidget(this);
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
    setCentralWidget(stack_);

    connect(pattern_, &PatternView::patternEdited, this, &MainWindow::refreshAll);
    connect(order_, &OrderView::edited, this, &MainWindow::refreshAll);
    connect(instrument_, &InstrumentView::edited, this, &MainWindow::refreshAll);
    connect(tables_, &TablesView::edited, this, &MainWindow::refreshAll);
    connect(songName_, &SongNameView::edited, this, &MainWindow::refreshSongInfo);

    auto *infoDock = new QDockWidget("Song", this);
    songInfo_ = new QLabel("<no song loaded>", infoDock);
    songInfo_->setMargin(8);
    songInfo_->setTextFormat(Qt::RichText);
    infoDock->setWidget(songInfo_);
    addDockWidget(Qt::RightDockWidgetArea, infoDock);

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
            editmode = mode; syncStack(); refreshStatus();
        });
    };
    addMode("&Pattern editor",   EDIT_PATTERN,    Qt::Key_F5);
    addMode("&Order/song editor", EDIT_ORDERLIST, Qt::Key_F6);
    addMode("&Instrument editor", EDIT_INSTRUMENT, Qt::Key_F7);
    addMode("&Tables editor",     EDIT_TABLES,    Qt::Key_F8);
    auto *namesA = modeMenu->addAction("Song&name editor");
    connect(namesA, &QAction::triggered, this, [this]{
        editmode = EDIT_NAMES; syncStack(); refreshStatus();
    });
    // Tab/Backtab: cycle edit modes. Use shortcut on MainWindow so any
    // focused subwidget yields to it.
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

    auto *playMenu = menuBar()->addMenu("&Play");
    auto *playA = playMenu->addAction("Play from &beginning");
    playA->setShortcut(Qt::Key_F1);
    connect(playA, &QAction::triggered, this, &MainWindow::playFromBeginning);
    auto *playPosA = playMenu->addAction("Play from &position");
    playPosA->setShortcut(Qt::Key_F2);
    connect(playPosA, &QAction::triggered, this, &MainWindow::playFromPos);
    auto *playPatA = playMenu->addAction("Play one pa&ttern");
    playPatA->setShortcut(Qt::Key_F3);
    connect(playPatA, &QAction::triggered, this, &MainWindow::playPattern);
    auto *stopA = playMenu->addAction("&Stop");
    stopA->setShortcut(Qt::Key_F4);
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
    tb->addAction(playA);
    tb->addAction(stopA);
    tb->addSeparator();
    tb->addAction(modeMenu->actions().at(0));
    tb->addAction(modeMenu->actions().at(1));
    tb->addAction(modeMenu->actions().at(2));
    tb->addAction(modeMenu->actions().at(3));

    statusOctave_ = new QLabel("Oct 2");
    statusInstr_ = new QLabel("Ins 01");
    statusMode_ = new QLabel("Pattern");
    statusFollow_ = new QLabel("Follow: off");
    statusBar()->addPermanentWidget(statusOctave_);
    statusBar()->addPermanentWidget(statusInstr_);
    statusBar()->addPermanentWidget(statusMode_);
    statusBar()->addPermanentWidget(statusFollow_);
    statusBar()->showMessage("Ready. Open a .sng to begin.");
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

void MainWindow::loadSongFile(const QString &path) {
    QByteArray ba = path.toLocal8Bit();
    std::strncpy(songfilename, ba.constData(), MAX_FILENAME - 1);
    songfilename[MAX_FILENAME - 1] = 0;
    loadsong();
    countpatternlengths();
    refreshAll();
    statusBar()->showMessage(QString("Loaded: %1").arg(path));
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
    if (savesong()) statusBar()->showMessage(QString("Saved: %1").arg(songfilename));
    else statusBar()->showMessage("Save failed");
}

void MainWindow::saveSongAs() {
    QString start = songfilename[0] ? QString::fromLocal8Bit(songfilename)
                                    : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getSaveFileName(this, "Save Song", start, "SNG (*.sng)");
    if (fn.isEmpty()) return;
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(songfilename, ba.constData(), MAX_FILENAME - 1);
    songfilename[MAX_FILENAME - 1] = 0;
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
    loadinstrument();
    refreshAll();
    statusBar()->showMessage(QString("Loaded ins: %1").arg(fn));
}

void MainWindow::saveInstrument() {
    QString start = instrfilename[0] ? QString::fromLocal8Bit(instrfilename)
                                     : QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString fn = QFileDialog::getSaveFileName(this, "Save Instrument", start, "INS (*.ins)");
    if (fn.isEmpty()) return;
    QByteArray ba = fn.toLocal8Bit();
    std::strncpy(instrfilename, ba.constData(), MAX_FILENAME - 1);
    instrfilename[MAX_FILENAME - 1] = 0;
    if (saveinstrument()) statusBar()->showMessage(QString("Saved ins: %1").arg(fn));
    else statusBar()->showMessage("Save instrument failed");
}

void MainWindow::refreshSongInfo() {
    songInfo_->setText(QString(
        "<b>Title:</b> %1<br>"
        "<b>Author:</b> %2<br>"
        "<b>Copyright:</b> %3<br>"
        "<b>Song:</b> %4 / 32<br>"
        "<b>SID:</b> %5<br>"
        "<b>Multiplier:</b> %6x"
    ).arg(QString::fromLocal8Bit(songname).toHtmlEscaped())
     .arg(QString::fromLocal8Bit(authorname).toHtmlEscaped())
     .arg(QString::fromLocal8Bit(copyrightname).toHtmlEscaped())
     .arg(esnum + 1)
     .arg(sidmodel ? "8580" : "6581")
     .arg(multiplier));
}

void MainWindow::refreshStatus() {
    statusOctave_->setText(QString("Oct %1").arg(epoctave));
    statusInstr_->setText(QString("Ins %1")
                          .arg(einum, 2, 16, QLatin1Char('0')).toUpper());
    static const char *modeNames[] = {"Pattern", "Order", "Instr", "Tables", "Names"};
    statusMode_->setText(modeNames[editmode]);
    statusFollow_->setText(followplay ? "Follow: ON" : "Follow: off");
}

void MainWindow::refreshAll() {
    pattern_->refresh();
    order_->refresh();
    instrument_->refresh();
    tables_->refresh();
    songName_->refresh();
    refreshSongInfo();
    refreshStatus();
}

void MainWindow::playFromBeginning() {
    initsong(esnum, PLAY_BEGINNING);
}
void MainWindow::playFromPos() {
    initsong(esnum, PLAY_POS);
}
void MainWindow::playPattern() {
    initsong(esnum, PLAY_PATTERN);
}
void MainWindow::stopSong() { stopsong(); }

void MainWindow::muteCurrentChannel() {
    mutechannel(epchn);
}

void MainWindow::prevMultiplierSlot() { prevmultiplier(); refreshSongInfo(); }
void MainWindow::nextMultiplierSlot() { nextmultiplier(); refreshSongInfo(); }
void MainWindow::toggleSidModel() {
    sidmodel ^= 1;
    refreshSongInfo();
}

void MainWindow::tick() {
    if (isplaying()) pattern_->refresh();
    refreshStatus();
}
