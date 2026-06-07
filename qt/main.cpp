#include <QApplication>
#include <QTimer>
#include <QStringList>
#include <QStyleFactory>
#include <QPalette>
#include <SDL/SDL.h>
#include <cstring>
#include "MainWindow.h"
#include "Rpc.h"
#include "PaAudio.h"
#include "Theme.h"
#include "Log.h"

extern "C" {
#include "gcommon.h"
#include "gsound.h"
#include "gplay.h"
void initchannels(void);
int sound_init(unsigned b, unsigned mr, unsigned writer, unsigned hardsid,
               unsigned m, unsigned ntsc, unsigned multiplier,
               unsigned catweasel, unsigned interpolate, unsigned customclockrate);
void sound_uninit(void);

extern unsigned b, mr, writer, hardsid, sidmodel, ntsc, multiplier;
extern unsigned catweasel, interpolate, customclockrate;
extern char songfilename[];
extern int songinit;
void loadsong(void);
void countpatternlengths(void);
void clearsong(int cs, int cp, int ci, int ct, int cn);
void sid_init(int speed, unsigned m, unsigned ntsc, unsigned interpolate,
              unsigned customclockrate, unsigned usefp);
}
#include "gplay.h" // PLAY_STOPPED

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    Log::init();
    qInfo("argv: argc=%d", argc);
    for (int i = 0; i < argc; i++) qInfo("  argv[%d]=%s", i, argv[i]);

    // Force Fusion style + an app-wide dark palette so the editor looks the
    // same across Linux / Windows / macOS. Native Windows style was
    // rendering QLineEdit / QSpinBox text in the default light-on-white
    // theme, making instrument fields unreadable.
    app.setStyle(QStyleFactory::create("Fusion"));
    QPalette pal;
    pal.setColor(QPalette::Window,          Theme::C::bgBase);
    pal.setColor(QPalette::WindowText,      Theme::C::text);
    pal.setColor(QPalette::Base,            Theme::C::bgBase);
    pal.setColor(QPalette::AlternateBase,   Theme::C::bgAlt);
    pal.setColor(QPalette::Text,            Theme::C::text);
    pal.setColor(QPalette::Button,          Theme::C::bgAlt);
    pal.setColor(QPalette::ButtonText,      Theme::C::text);
    pal.setColor(QPalette::Highlight,       Theme::C::editRow);
    pal.setColor(QPalette::HighlightedText, Theme::C::text);
    pal.setColor(QPalette::ToolTipBase,     Theme::C::bgAlt);
    pal.setColor(QPalette::ToolTipText,     Theme::C::text);
    pal.setColor(QPalette::PlaceholderText, Theme::C::textDim);
    pal.setColor(QPalette::Disabled, QPalette::Text,       Theme::C::textDim);
    pal.setColor(QPalette::Disabled, QPalette::ButtonText, Theme::C::textDim);
    app.setPalette(pal);

    // Filter our own flags so Qt's platform parser doesn't choke.
    bool rpcMode = false;
    QString songArg;
    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); i++) {
        if (args[i] == "--rpc") rpcMode = true;
        else if (!args[i].startsWith("-")) songArg = args[i];
    }

    // SDL audio init removed — QAudioSink owns the audio device now. SDL is
    // only kept around for misc helpers in the bundled bme code, so init the
    // timer subsystem (cheap, no device locking).
    if (SDL_Init(SDL_INIT_TIMER) < 0) {
        qWarning("SDL_Init timer failed: %s", SDL_GetError());
    }

    initchannels();
    songinit = PLAY_STOPPED;
    // Initialise libresidfp directly. Stick with DECIMATE (interpolate=0)
    // for the audio thread — RESAMPLE's sinc filter is 5-10× slower and was
    // causing the audio thread to miss its real-time deadline, producing
    // the 'instruments not playing on time' stutter the user reported. The
    // ticks RESAMPLE was supposed to fix turned out to be the underrun
    // memset(0) (now sample-hold) and not aliasing.
    sid_init((int)mr, sidmodel, ntsc, /*interpolate=*/0, customclockrate, 1);

    PaAudio audio;
    if (!audio.start((int)mr)) {
        qWarning("PaAudio failed to start; running silent.");
    }

    // Start with a fresh empty song so the pattern editor is immediately
    // usable (default 64-row patterns, RST endmark on each channel). The SDL
    // build does this implicitly via the main loop init; the Qt build skipped
    // it and left pattlen[] zeroed.
    clearsong(1, 1, 1, 1, 1);
    countpatternlengths();

    MainWindow w;
    w.show();
    if (!songArg.isEmpty()) w.loadSongFile(songArg);

    Rpc *rpc = nullptr;
    if (rpcMode) rpc = new Rpc(&w);

    int rc = app.exec();

    delete rpc;
    audio.stop();
    SDL_Quit();
    return rc;
}
