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
unsigned sid2model = 0;  // Second SID chip (stereo mode only).
int stereo_mode = 0;     // 0 = mono (3 channels, SID1 only). 1 = stereo
                         // (6 channels, SID1 + SID2). Runtime-switchable.
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
float equaldivisionsperoctave = 12.0f;
int tuningcount = 0;
double tuning[96];
char specialnotenames[186];
char scalatuningfilepath[MAX_PATHNAME];
char tuningname[64];

/* Note-name lookup table. Originally lives in src/gdisplay.c (which we
   don't link in the Qt build because it pulls SDL render code). The C
   side mutates this when -J / -Y / Scala / setspecialnotenames() runs;
   the Qt PatternView / OrderView read it via an extern. Sized at 96 so
   setspecialnotenames() can safely fill up to 93 entries plus 3 markers. */
char *notename[96] = {
  "C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "...", "---", "+++"
};

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

/* converthex() compares tolower(key) against this table, so it must hold
 * the ASCII chars '0'..'9','a'..'f' — not raw nibble values. The previous
 * raw-int table never matched anything, so hexnybble stayed at -1 and the
 * pattern instr / cmd / param digits in the editor were unwritable. */
unsigned char hexkeytbl[16] = {'0','1','2','3','4','5','6','7',
                               '8','9','a','b','c','d','e','f'};

/* Re-implementation of the SDL build's converthex() — pulled in so the Qt
 * keypress handler can resolve a hex key into a nibble before calling
 * patterncommands(). Body matches src/goattrk2.c:575. */
#include <ctype.h>
extern int hexnybble;
extern int key;
extern int shiftpressed;
void converthex(void) {
    int c;
    hexnybble = -1;
    for (c = 0; c < 16; c++) {
        if (tolower(key) == hexkeytbl[c]) {
            if (c >= 10) {
                if (!shiftpressed) hexnybble = c;
            } else {
                hexnybble = c;
            }
        }
    }
}

/* ----- Microtonal helpers (backport from v2.75) -----
   These live here for the Qt build because src/goattrk2.c (where they
   live in the SDL build) is not linked: it owns main() + console UI. */

#include <math.h>
#include "gplay.h"   /* freqtbllo / freqtblhi */

/* Restorable defaults — used by resetnotenames() to undo a -J/Scala rename
   without losing the original labels. */
static const char *defaultnotenames[96] =
 {"C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
  "C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
  "C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
  "C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
  "C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
  "C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
  "C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
  "C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "...", "---", "+++"};

void resetnotenames(void)
{
  int i;
  for (i = 0; i < 96; i++)
    notename[i] = (char *)defaultnotenames[i];
}

void calculatefreqtable(void)
{
  /* Backport from v2.75+: supports equal-divisions-per-octave (-Q) and
     Scala-style ratio tunings (loaded via readscalatuningfile()).
     basepitch=0 means "use built-in 12-TET table baked into gplay.c"; in
     that case we still recompute when an alt tuning is requested, using
     440Hz as the reference so the user gets audible detuning. */
  float bp = basepitch > 0.0f ? basepitch : 440.0f;
  double basefreq = (double)bp * (16777216.0 / 985248.0) * pow(2.0, 0.25) / 32.0;
  double cyclebasefreq = basefreq;
  double freq = basefreq;
  int c;
  int i;

  if (tuningcount)
  {
    c = 0;
    while (c < 96)
    {
      for (i = 0; i < tuningcount; i++)
      {
        if (c < 96)
        {
          int intfreq = freq + 0.5;
          if (intfreq > 0xffff) intfreq = 0xffff;
          freqtbllo[c] = intfreq & 0xff;
          freqtblhi[c] = intfreq >> 8;
          freq = cyclebasefreq * tuning[i];
          c++;
        }
      }
      cyclebasefreq = freq;
    }
  }
  else
  {
    for (c = 0; c < 8*12 ; c++)
    {
      double note = c;
      double f = basefreq * pow(2.0, note/(double)equaldivisionsperoctave);
      int intfreq = f + 0.5;
      if (intfreq > 0xffff) intfreq = 0xffff;
      freqtbllo[c] = intfreq & 0xff;
      freqtblhi[c] = intfreq >> 8;
    }
  }
}

void setspecialnotenames(void)
{
  /* Map every two chars of specialnotenames[] to one note in a cycle,
     suffixed with the octave digit. */
  int i = 0, j, oct = 0;
  char *name;
  char octave[11];

  while (i < 93)
  {
    int wrapped = 1;
    for (j = 0; j < 186; j += 2)
    {
      if (specialnotenames[j] == '\0') break;
      if (i < 93)
      {
        name = (char *)malloc(4);
        if (!name) return;
        strncpy(name, specialnotenames + j, 2);
        sprintf(octave, "%d", oct);
        strcpy(name + 2, octave);
        notename[i] = name;
        i++;
        wrapped = 0;
      }
    }
    if (wrapped) break;  /* empty specialnotenames — don't infinite loop */
    oct++;
  }
}

void readscalatuningfile(void)
{
  /* Scala .scl tuning file parser. See readme + Scala format spec. */
  FILE *scalatuningfile;
  char *configptr;
  char strbuf[64];
  char name[3];
  int i;
  double numerator;
  double denominator;
  double centvalue;

  scalatuningfile = fopen(scalatuningfilepath, "rt");
  if (!scalatuningfile) return;

  /* Tuning name */
  for (;;)
  {
    if (feof(scalatuningfile)) { fclose(scalatuningfile); return; }
    fgets(configbuf, MAX_PATHNAME, scalatuningfile);
    if ((configbuf[0]) && (configbuf[0] != '!') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
  }
  configptr = configbuf;
  sscanf(configptr, "%63[^\t\n]", tuningname);

  /* Tuning count */
  for (;;)
  {
    if (feof(scalatuningfile)) { fclose(scalatuningfile); return; }
    fgets(configbuf, MAX_PATHNAME, scalatuningfile);
    if ((configbuf[0]) && (configbuf[0] != '!') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
  }
  configptr = configbuf;
  sscanf(configptr, "%d", &tuningcount);
  if (tuningcount < 1) tuningcount = 0;
  if (tuningcount > 96) tuningcount = 96;

  /* Reset note names accumulator. We'll rebuild from any letter pairs
     present on each ratio line. */
  specialnotenames[0] = '\0';

  /* Tunings */
  for (i = 0; i < tuningcount; i++)
  {
    for (;;)
    {
      if (feof(scalatuningfile)) { fclose(scalatuningfile); return; }
      fgets(configbuf, MAX_PATHNAME, scalatuningfile);
      if ((configbuf[0]) && (configbuf[0] != '!') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
    }
    configptr = configbuf;
    name[0] = '\0';
    strbuf[0] = '\0';
    sscanf(configptr, "%63s %2s", strbuf, name);
    if (!i)
    {
      strcpy(specialnotenames, name);
    }
    else
    {
      if (i == tuningcount - 1)
      {
        char *tmp = strdup(specialnotenames);
        strcpy(specialnotenames, name);
        if (tmp) { strcat(specialnotenames, tmp); free(tmp); }
      }
      else
      {
        strcat(specialnotenames, name);
      }
    }
    if (!strchr(strbuf, '.'))
    {
      sscanf(strbuf, "%lf", &numerator);
      if (strchr(strbuf, '/'))
      {
        sscanf(strchr(strbuf, '/') + 1, "%lf", &denominator);
        if (denominator != 0.0) tuning[i] = numerator / denominator;
      }
    }
    else
    {
      sscanf(configptr, "%lf", &centvalue);
      tuning[i] = pow(2.0, centvalue / 1200.0);
    }
  }
  fclose(scalatuningfile);
}
