#ifndef GSID_H
#define GSID_H

#define NUMSIDREGS 0x19
#define SIDWRITEDELAY 9 // lda $xxxx,x 4 cycles, sta $d400,x 5 cycles
#define SIDWAVEDELAY 4 // and $xxxx,x 4 cycles extra

/*
 * FILTERPARAMS is preserved verbatim for binary stability of the
 * GoatTracker config file (goattrk2.cfg). goattrk2.c reads/writes
 * each field in order, so the layout MUST NOT change.
 *
 * Since the libresidfp backend replaces the old reSID + reSID-fp pair
 * the following fields are now INERT - they are still parsed from /
 * written to the config file but have no audible effect:
 *
 *   distortionrate
 *   distortionpoint
 *   distortioncfthreshold
 *   type3baseresistance
 *   type3offset
 *   type3steepness
 *   type3minimumfetresistance
 *   type4k
 *   type4b
 *   voicenonlinearity
 *
 * The old reSID-fp distortion / type3 / type4 / voice nonlinearity
 * tuning has no direct equivalent in libresidfp 2.x; that backend
 * exposes only setFilter6581Curve / setFilter6581Range /
 * setFilter8580Curve / enableOld6581caps.
 */
typedef struct
{
  float distortionrate;
  float distortionpoint;
  float distortioncfthreshold;
  float type3baseresistance;
  float type3offset;
  float type3steepness;
  float type3minimumfetresistance;
  float type4k;
  float type4b;
  float voicenonlinearity;
} FILTERPARAMS;

#ifdef __cplusplus
extern "C" {
#endif

void sid_init(int speed, unsigned m, unsigned ntsc, unsigned interpolate, unsigned customclockrate, unsigned usefp);
int sid_fillbuffer(short *ptr, int samples);
unsigned char sid_getorder(unsigned char index);
void sid_getlevels(unsigned char *out);

#ifdef __cplusplus
}
#endif

#ifndef GSID_C
extern unsigned char sidreg[NUMSIDREGS];
extern FILTERPARAMS filterparams;
#endif

#endif
