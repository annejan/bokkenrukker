// Globals normally defined in src/goattrk2.c
// We exclude goattrk2.c (has main + UI) but the linked core .c files
// (gpattern, gorder, ginstr, gtable, gplay, gsong, gsound) reference these
// via extern declarations in goattrk2.h.

#define GOATTRK2_C
#include "goattrk2.h"

int menu = 0;
int editmode = EDIT_PATTERN;
int recordmode = 1;
int followplay = 0;
int hexnybble = -1;
int stepsize = 4;
int autoadvance = 0;
int defaultpatternlength = 64;
int cursorflash = 0;
int cursorcolortable[] = {1,2,7,2};
int exitprogram = 0;
int eacolumn = 0;
int eamode = 0;

unsigned keypreset = KEY_TRACKER;
unsigned playerversion = 0;
int fileformat = 0;
int zeropageadr = 0xfc;
int playeradr = 0x1000;
unsigned sidmodel = 0;
unsigned multiplier = 1;
unsigned adparam = 0x0f00;
unsigned ntsc = 0;
unsigned patternhex = 0;
unsigned sidaddress = 0xd400;
unsigned finevibrato = 1;
unsigned optimizepulse = 1;
unsigned optimizerealtime = 1;
unsigned customclockrate = 0;
unsigned usefinevib = 0;
unsigned b = 100;
unsigned mr = 44100;
unsigned writer = 0;
unsigned hardsid = 0;
unsigned catweasel = 0;
unsigned interpolate = 0;
unsigned residdelay = 0;
unsigned hardsidbufinteractive = 20;
unsigned hardsidbufplayback = 400;
float basepitch = 0.0f;

char configbuf[MAX_PATHNAME];
char loadedsongfilename[MAX_FILENAME];
char songfilename[MAX_FILENAME];
char songfilter[MAX_FILENAME] = "*.sng";
char songpath[MAX_PATHNAME];
char instrfilename[MAX_FILENAME];
char instrfilter[MAX_FILENAME] = "*.ins";
char instrpath[MAX_PATHNAME];
char packedpath[MAX_PATHNAME];

char *programname = "GoatTracker Qt";
char textbuffer[MAX_PATHNAME];

unsigned char hexkeytbl[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
