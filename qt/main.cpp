#include <QApplication>
#include <QTimer>
#include <QStringList>
#include <SDL/SDL.h>
#include <cstring>
#include "MainWindow.h"
#include "Rpc.h"
#include "QtAudio.h"

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

    // Filter our own flags so Qt's platform parser doesn't choke.
    bool rpcMode = false;
    QString songArg;
    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); i++) {
        if (args[i] == "--rpc") rpcMode = true;
        else if (!args[i].startsWith("-")) songArg = args[i];
    }

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        qWarning("SDL_Init audio failed: %s", SDL_GetError());
    }

    initchannels();
    songinit = PLAY_STOPPED;
    // Initialise libresidfp directly (skip bme/SDL audio init).
    sid_init((int)mr, sidmodel, ntsc, interpolate, customclockrate, 1);

    QtAudio audio;
    if (!audio.start((int)mr)) {
        qWarning("QtAudio failed to start; running silent.");
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
