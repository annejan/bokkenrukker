/*
 * GOATTRACKER libresidfp interface
 *
 * Replaces the previous gsid.cpp that drove the bundled reSID 0.16 and
 * reSID-fp engines. The single backend now is reSIDfp from libresidfp.
 */

#define GSID_C

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "residfp/residfp.h"
#include "residfp/residfp_defs.h"

extern "C" {

#include "gcommon.h"
#include "gsid.h"
#include "gsound.h"

int clockrate;
int samplerate;
unsigned char sidreg[NUMSIDREGS];
unsigned char sidreg2[NUMSIDREGS];
unsigned char sidorder[] =
  {0x15,0x16,0x18,0x17,
   0x05,0x06,0x02,0x03,0x00,0x01,0x04,
   0x0c,0x0d,0x09,0x0a,0x07,0x08,0x0b,
   0x13,0x14,0x10,0x11,0x0e,0x0f,0x12};

unsigned char altsidorder[] =
  {0x15,0x16,0x18,0x17,
   0x04,0x00,0x01,0x02,0x03,0x05,0x06,
   0x0b,0x07,0x08,0x09,0x0a,0x0c,0x0d,
   0x12,0x0e,0x0f,0x10,0x11,0x13,0x14};

/*
 * FILTERPARAMS retained for binary stability of the config file IO in
 * goattrk2.c (lines ~137-146 read, ~449-460 write). The libresidfp
 * backend does NOT consume distortion / type3 / type4 / voicenonlinearity
 * tuning - those were specific to the old reSID-fp distortion filter
 * model. Values are read and written so the config file stays compatible,
 * but they have no audible effect any more. See README / report.
 */
FILTERPARAMS filterparams =
  {0.50f, 3.3e6f, 1.0e-4f,
   1147036.4394268463f, 274228796.97550374f, 1.0066634233403395f, 16125.154840564108f,
   5.5f, 20.f,
   0.9613160610660189f};

extern unsigned residdelay;
extern unsigned adparam;

/* SID1 always exists; SID2 lazy-instantiated when the runtime stereo_mode
 * flag flips on. extern reference here, defined in qt_globals.c. */
extern "C" int stereo_mode;
static reSIDfp::residfp *sid = 0;
static reSIDfp::residfp *sid2 = 0;

void sid_init(int speed, unsigned m, unsigned ntsc, unsigned interpolate, unsigned customclockrate, unsigned usefp)
{
  int c;
  (void)usefp; /* dual-backend toggle gone; reSIDfp always used */

  if (ntsc) clockrate = NTSCCLOCKRATE;
    else clockrate = PALCLOCKRATE;

  if (customclockrate)
    clockrate = customclockrate;

  samplerate = speed;

  if (!sid)
    sid = new reSIDfp::residfp();
  if (stereo_mode) {
    if (!sid2) sid2 = new reSIDfp::residfp();
  } else if (sid2) {
    // Tear down SID2 when stereo flips off so audio actually returns to a
    // single-SID mix and CPU load drops.
    delete sid2;
    sid2 = 0;
  }

  /* Chip model: SID1 follows m, SID2 follows sid2model independently. */
  if (m == 1)
    sid->setChipModel(reSIDfp::CSG8580);
  else
    sid->setChipModel(reSIDfp::MOS6581);
  if (sid2) {
    extern unsigned sid2model;
    if (sid2model == 1)
      sid2->setChipModel(reSIDfp::CSG8580);
    else
      sid2->setChipModel(reSIDfp::MOS6581);
  }

  /*
   * interpolate values used by the old code:
   *   0 -> SAMPLE_FAST / SAMPLE_INTERPOLATE (low quality)
   *   1 -> SAMPLE_INTERPOLATE / SAMPLE_RESAMPLE_INTERPOLATE (high quality)
   *
   * libresidfp 2.x exposes only DECIMATE / RESAMPLE / NONE. Map:
   *   0 -> DECIMATE  (fast, lower quality)
   *   else -> RESAMPLE (sinc, good quality)
   */
  reSIDfp::SamplingMethod method =
      (interpolate == 0) ? reSIDfp::DECIMATE : reSIDfp::RESAMPLE;

  sid->setSamplingParameters((double)clockrate, method, (double)speed);
  sid->reset();
  sid->enableFilter(true);
  if (sid2) {
    sid2->setSamplingParameters((double)clockrate, method, (double)speed);
    sid2->reset();
    sid2->enableFilter(true);
  }
  for (c = 0; c < NUMSIDREGS; c++)
  {
    sidreg[c] = 0x00;
    sidreg2[c] = 0x00;
  }
}

unsigned char sid_getorder(unsigned char index)
{
  if (adparam >= 0xf000)
    return altsidorder[index];
  else
    return sidorder[index];
}

void sid_getlevels(unsigned char *out)
{
  for (int i = 0; i < MAX_CHN; i++) out[i] = 0;
  if (!sid) return;
  out[0] = sid->envelopeLevel(0);
  out[1] = sid->envelopeLevel(1);
  out[2] = sid->envelopeLevel(2);
  if (sid2) {
    out[3] = sid2->envelopeLevel(0);
    out[4] = sid2->envelopeLevel(1);
    out[5] = sid2->envelopeLevel(2);
  }
}

/*
 * Render `samples` mono 16-bit samples into `ptr`.
 *
 * The old reSID API had clock(cycles, buf, maxsamples) which would
 * stop early if maxsamples filled. libresidfp's clock(cycles, buf)
 * simply runs the requested cycles and returns however many output
 * samples landed (the internal resampler buffers fractional cycles).
 * To honour the old "rewrite SID regs at known cycle offsets" timing
 * we run small bursts between writes, accumulate produced samples,
 * and only top up at the end if the requested count was not yet hit.
 */
// Render one SID instance into `ptr` for `samples` mono samples. Body is the
// historical sid_fillbuffer logic parameterised over the residfp instance
// and the register shadow array, so we can drive SID2 the same way for the
// stereo mix path below.
static int render_sid(reSIDfp::residfp *s, const unsigned char *regs,
                      short *ptr, int samples) {
  int tdelta;
  int tdelta2;
  int result = 0;
  int total = 0;
  int c;

  if (!s) return 0;

  int badline = rand() % NUMSIDREGS;

  tdelta = clockrate * samples / samplerate;
  if (tdelta <= 0) return total;

  for (c = 0; c < NUMSIDREGS; c++)
  {
    unsigned char o = sid_getorder(c);

    /* Extra delay for loading the waveform (and mt_chngate,x) */
    if ((o == 4) || (o == 11) || (o == 18))
    {
      tdelta2 = SIDWAVEDELAY;
      if (samples > 0)
      {
        result = s->clock((unsigned)tdelta2, ptr);
        if (result > samples) result = samples;
        total += result;
        ptr += result;
        samples -= result;
      }
      else
      {
        s->clockSilent((unsigned)tdelta2);
      }
      tdelta -= SIDWAVEDELAY;
    }

    /* Possible random badline delay once per writing */
    if ((badline == c) && (residdelay))
    {
      tdelta2 = (int)residdelay;
      if (samples > 0)
      {
        result = s->clock((unsigned)tdelta2, ptr);
        if (result > samples) result = samples;
        total += result;
        ptr += result;
        samples -= result;
      }
      else
      {
        s->clockSilent((unsigned)tdelta2);
      }
      tdelta -= (int)residdelay;
    }

    s->write(o, regs[o]);

    tdelta2 = SIDWRITEDELAY;
    if (samples > 0)
    {
      result = s->clock((unsigned)tdelta2, ptr);
      if (result > samples) result = samples;
      total += result;
      ptr += result;
      samples -= result;
    }
    else
    {
      s->clockSilent((unsigned)tdelta2);
    }
    tdelta -= SIDWRITEDELAY;

    if (tdelta <= 0 && samples <= 0) return total;
  }

  if (tdelta > 0 && samples > 0)
  {
    result = s->clock((unsigned)tdelta, ptr);
    if (result > samples) result = samples;
    total += result;
    ptr += result;
    samples -= result;
  }

  /* Loop extra cycles until all samples produced */
  while (samples > 0)
  {
    tdelta = clockrate * samples / samplerate;
    if (tdelta <= 0) return total;

    result = s->clock((unsigned)tdelta, ptr);
    if (result > samples) result = samples;
    if (result <= 0) break;
    total += result;
    ptr += result;
    samples -= result;
  }

  return total;
}

int sid_fillbuffer(short *ptr, int samples)
{
  if (!sid) return 0;
  if (!sid2) return render_sid(sid, sidreg, ptr, samples);

  // Stereo path. Use the same per-SID renderer that produces the mono path's
  // sample count — that loop has an aggressive top-up at the tail, so both
  // calls reliably hit `samples` and we mix N==samples both sides.
  static std::vector<short> tmp;
  if ((int)tmp.size() < samples) tmp.resize(samples);

  int n1 = render_sid(sid,  sidreg,  ptr,         samples);
  int n2 = render_sid(sid2, sidreg2, tmp.data(),  samples);
  int n  = std::min(n1, n2);

  // Straight sum with hard clip. SID2 with all-zero registers produces a
  // near-silent stream so this keeps the SID1 voicing at full volume in the
  // common case (no extra parts written to ch4-6). When SID2 actually has
  // material, peaks may clip — that's preferable to permanent attenuation.
  for (int i = 0; i < n; i++) {
    int v = (int)ptr[i] + (int)tmp[i];
    if (v >  32767) v =  32767;
    if (v < -32768) v = -32768;
    ptr[i] = (short)v;
  }
  return n;
}

}
