#include "PaAudio.h"

#include <portaudio.h>

#include <QThread>
#include <QDebug>
#include <cstring>

extern "C" {
#include "gcommon.h"
#include "gplay.h"
int  sid_fillbuffer(short *ptr, int samples);
extern unsigned multiplier;
extern unsigned ntsc;
void stopsong(void);
extern int songinit;
}

PaAudio *PaAudio::self_ = nullptr;

PaAudio::PaAudio() { self_ = this; }
PaAudio::~PaAudio() { stop(); if (self_ == this) self_ = nullptr; }

int PaAudio::paCallback(const void * /*in*/, void *out, unsigned long frames,
                        const PaStreamCallbackTimeInfo * /*t*/,
                        unsigned long /*flags*/, void *user) {
    auto *self = static_cast<PaAudio*>(user);
    short *o = static_cast<short*>(out);

    // Hard-fenced by the UI thread? Silence chunk + return — cheaper than
    // stalling the device. The fence is only set during the few ms a
    // sid_init / loadsong takes on the UI thread.
    if (self->fenced.load(std::memory_order_acquire)) {
        std::memset(o, 0, frames * sizeof(short));
        return 0; // paContinue
    }

    const int frameHz = (ntsc ? 60 : 50) * (multiplier ? multiplier : 1);
    const double samplesPerTickF = (double)self->sampleRate_ / (double)frameHz;

    unsigned long produced = 0;
    while (produced < frames) {
        if (self->sampleAccumF_ <= 0.0) {
            playroutine();
            self->sampleAccumF_ += samplesPerTickF;
        }
        long chunk = (long)self->sampleAccumF_;
        if (chunk <= 0) chunk = 1;
        if (chunk > (long)(frames - produced)) chunk = (long)(frames - produced);
        int got = sid_fillbuffer(o + produced, (int)chunk);
        if (got <= 0) {
            short hold = (produced > 0) ? o[produced - 1] : 0;
            for (long i = 0; i < chunk; i++) o[produced + i] = hold;
            produced += chunk;
            self->sampleAccumF_ -= (double)chunk;
            continue;
        }
        produced += (unsigned long)got;
        self->sampleAccumF_ -= (double)got;
    }
    return 0;
}

bool PaAudio::start(int sampleRate) {
    sampleRate_ = sampleRate;

    PaError e = Pa_Initialize();
    if (e != paNoError) {
        qWarning("PaAudio: Pa_Initialize failed: %s", Pa_GetErrorText(e));
        return false;
    }
    paInited_ = true;

    // paFramesPerBufferUnspecified -> PortAudio picks the host-optimal frame
    // count, which on ALSA/Pulse is typically ~256-512 samples (~5-10 ms at
    // 44.1 kHz). That puts the editor playhead inside one C64 frame of the
    // audio, no compensation ring needed.
    e = Pa_OpenDefaultStream(&stream_,
                             0,            // no input
                             1,            // mono out
                             paInt16,
                             (double)sampleRate,
                             paFramesPerBufferUnspecified,
                             &PaAudio::paCallback,
                             this);
    if (e != paNoError) {
        qWarning("PaAudio: Pa_OpenDefaultStream failed: %s", Pa_GetErrorText(e));
        Pa_Terminate();
        paInited_ = false;
        return false;
    }

    e = Pa_StartStream(stream_);
    if (e != paNoError) {
        qWarning("PaAudio: Pa_StartStream failed: %s", Pa_GetErrorText(e));
        Pa_CloseStream(stream_);
        stream_ = nullptr;
        Pa_Terminate();
        paInited_ = false;
        return false;
    }

    const PaStreamInfo *info = Pa_GetStreamInfo(stream_);
    if (info) {
        qInfo("PaAudio: started — rate=%g Hz, output latency=%.1f ms",
              info->sampleRate, info->outputLatency * 1000.0);
    }
    return true;
}

void PaAudio::stop() {
    if (stream_) {
        Pa_StopStream(stream_);
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
    if (paInited_) {
        Pa_Terminate();
        paInited_ = false;
    }
}

AudioFence::AudioFence() {
    auto *a = PaAudio::instance();
    if (a) {
        a->fenced.store(true, std::memory_order_release);
        // Wait long enough for any in-flight PortAudio callback to drain.
        // Callback chunks are typically <10 ms at the default buffer size;
        // 5 ms covers the common case and is cheap if the audio thread is
        // already past the callback boundary.
        QThread::msleep(5);
    }
    stopsong();
    songinit = PLAY_STOPPED;
}
AudioFence::~AudioFence() {
    auto *a = PaAudio::instance();
    if (a) a->fenced.store(false, std::memory_order_release);
}
