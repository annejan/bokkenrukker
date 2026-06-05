#include <QApplication>
#include <QTimer>
#include <SDL/SDL.h>
#include <cstring>
#include "MainWindow.h"

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
void loadsong(void);
void countpatternlengths(void);
}

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        qWarning("SDL_Init audio failed: %s", SDL_GetError());
    }

    initchannels();
    if (!sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier,
                    catweasel, interpolate, customclockrate)) {
        qWarning("sound_init failed; running silent.");
    }

    MainWindow w;
    w.show();

    if (argc > 1) {
        w.loadSongFile(QString::fromLocal8Bit(argv[1]));
    }

    int rc = app.exec();

    sound_uninit();
    SDL_Quit();
    return rc;
}
