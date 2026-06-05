// Stubs for symbols normally provided by goatdata.c (compiled-in datafile)
// and any UI/render functions the linked core may reference.
//
// We skip the bme datafile entirely. io_open() falls back to fopen()
// when no datafile is opened, so chargen.bin/palette.bin/cursor.bin
// requests will simply fail — and we never make them since we don't
// link gconsole.c.

#include <stddef.h>

// Empty datafile so io_openlinkeddatafile (if anyone calls it) is harmless.
unsigned char datafile[1] = {0};

// Globals normally defined in gconsole.c (input state).
int key = 0;
int rawkey = 0;
int shiftpressed = 0;
int cursorflashdelay = 0;
int mouseb = 0;
int prevmouseb = 0;
int mouseheld = 0;
int mousex = 0;
int mousey = 0;

// Globals normally defined in greloc.c (used by gtable/gsong via includes).
#include "gcommon.h"
unsigned char pattused[MAX_PATT];
unsigned char instrused[MAX_INSTR];
unsigned char tableused[MAX_TABLES][MAX_TABLELEN+1];
int tableerror = 0;

// Stub for editstring (lives in gfile.c, used by gorder.c name editor).
void editstring(char *buffer, int maxlength) { (void)buffer; (void)maxlength; }

// Timer hooks from gdisplay.c, called by gplay.c.
void resettime(void) {}
void incrementtime(void) {}

// Speed multiplier controls — originally in goattrk2.c.
extern unsigned multiplier, b, mr, writer, hardsid, sidmodel, ntsc, catweasel, interpolate, customclockrate;
int sound_init(unsigned b, unsigned mr, unsigned writer, unsigned hardsid,
               unsigned m, unsigned ntsc, unsigned multiplier,
               unsigned catweasel, unsigned interpolate, unsigned customclockrate);
void prevmultiplier(void) {
    if (multiplier > 0) {
        multiplier--;
        sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier,
                   catweasel, interpolate, customclockrate);
    }
}
void nextmultiplier(void) {
    if (multiplier < 16) {
        multiplier++;
        sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier,
                   catweasel, interpolate, customclockrate);
    }
}

