#include "QtAudio.h"

#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QIODevice>
#include <QMutexLocker>
#include <QDebug>
#include <cstring>

extern "C" {
#include "gcommon.h"
#include "gplay.h"  // PLAY_STOPPED + playroutine prototype
int  sid_fillbuffer(short *ptr, int samples);
extern unsigned multiplier;
extern unsigned ntsc;
void stopsong(void);
extern int songinit;
}

// Pull-mode QIODevice that the QAudioSink calls into whenever its internal
// buffer needs more samples. Drives the C playroutine at the C64 frame rate
// (PAL 50 Hz / NTSC 60 Hz times the user's speed multiplier) and the
// libresidfp render via sid_fillbuffer.
class QtAudio::PullDevice : public QIODevice {
public:
    PullDevice(int sampleRate, QObject *parent = nullptr)
        : QIODevice(parent), sampleRate_(sampleRate) {
        open(QIODevice::ReadOnly);
    }
    bool isSequential() const override { return true; }
    qint64 readData(char *data, qint64 maxlen) override {
        // Lock the QtAudio mutex for the whole fill so a UI-thread loadsong
        // can't rewrite chn[] / sidreg[] / songorder[] mid-buffer. The fill
        // is bounded by maxlen so the worst-case lock-hold is one Qt audio
        // chunk (~20 ms at 200 ms buffer).
        QMutexLocker lk(&QtAudio::instance()->mutex());
        auto *self = QtAudio::instance();
        // Use sub-sample-precise fractional accumulator so the playroutine
        // fires at exactly the requested frame rate regardless of the
        // sampleRate / frame_hz ratio not dividing evenly.
        const int frame = (ntsc ? 60 : 50) * (multiplier ? multiplier : 1);
        const double samplesPerTickF = (double)sampleRate_ / (double)frame;
        short *out = reinterpret_cast<short*>(data);
        const qint64 frames = maxlen / 2;
        qint64 produced = 0;
        while (produced < frames) {
            if (sampleAccumF_ <= 0.0) {
                playroutine();
                self->pushSnapshot(samplesProduced_);
                sampleAccumF_ += samplesPerTickF;
            }
            qint64 chunk = (qint64)sampleAccumF_;
            if (chunk <= 0) chunk = 1;
            if (chunk > frames - produced) chunk = frames - produced;
            const int got = sid_fillbuffer(out + produced, (int)chunk);
            if (got <= 0) {
                short hold = (produced > 0) ? out[produced - 1] : 0;
                for (qint64 i = 0; i < chunk; i++) out[produced + i] = hold;
                produced += chunk;
                sampleAccumF_ -= (double)chunk;
                continue;
            }
            produced += got;
            sampleAccumF_ -= (double)got;
        }
        samplesProduced_ += produced;
        return produced * 2;
    }
    qint64 writeData(const char *, qint64) override { return -1; }
    qint64 bytesAvailable() const override {
        return std::numeric_limits<qint64>::max();
    }
    qint64 samplesProduced() const { return samplesProduced_; }
private:
    int     sampleRate_;
    double  sampleAccumF_ = 0.0;
    qint64  samplesProduced_ = 0;
};

QtAudio *QtAudio::self_ = nullptr;

QtAudio::QtAudio(QObject *parent) : QObject(parent) { self_ = this; }
QtAudio::~QtAudio() { stop(); if (self_ == this) self_ = nullptr; }

extern "C" {
extern CHN chn[];
}

void QtAudio::pushSnapshot(qint64 sampleAt) {
    Tick &t = ring_[ringHead_];
    t.sampleAt = sampleAt;
    for (int c = 0; c < MAX_CHN; c++) {
        t.snap.pattptr[c] = (int)chn[c].pattptr;
        t.snap.songptr[c] = (int)chn[c].songptr;
        t.snap.instr  [c] = (int)chn[c].instr;
    }
    ringHead_ = (ringHead_ + 1) % kRingN;
    if (ringFill_ < kRingN) ringFill_++;
}

QtAudio::PlaySnapshot QtAudio::currentSnapshot() {
    PlaySnapshot out{};
    // No sink running -> fall back to the live chn[]; better than zeros.
    if (!sink_ || ringFill_ == 0) {
        for (int c = 0; c < MAX_CHN; c++) {
            out.pattptr[c] = (int)chn[c].pattptr;
            out.songptr[c] = (int)chn[c].songptr;
            out.instr  [c] = (int)chn[c].instr;
        }
        return out;
    }
    // Audible sample position = how many samples the audio device has
    // actually pushed to the speaker. processedUSecs covers buffered-but-
    // not-yet-played samples too on some backends; QAudioSink semantics
    // are "samples the device has consumed from us", which is what we want.
    qint64 playedSamples = (qint64)sink_->processedUSecs()
                            * (qint64)sampleRate_ / 1000000LL;
    QMutexLocker lk(&mtx_);
    // Walk back from the most recent tick to find the latest one whose
    // sampleAt <= playedSamples. ringHead_ points at the *next* write slot,
    // so the freshest entry is at (ringHead_ - 1).
    int best = -1;
    for (int i = 0; i < ringFill_; i++) {
        int idx = (ringHead_ - 1 - i + kRingN) % kRingN;
        if (ring_[idx].sampleAt <= playedSamples) { best = idx; break; }
    }
    if (best < 0) {
        // playedSamples is older than every snapshot we still hold —
        // return the oldest one we have.
        best = (ringHead_ - ringFill_ + kRingN) % kRingN;
    }
    return ring_[best].snap;
}

void QtAudio::suspend() {
    if (sink_) sink_->suspend();
}
void QtAudio::resume() {
    if (sink_) sink_->resume();
}

bool QtAudio::start(int sampleRate) {
    sampleRate_ = sampleRate;
    QAudioFormat fmt;
    fmt.setSampleRate(sampleRate);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);

    QAudioDevice out = QMediaDevices::defaultAudioOutput();
    if (!out.isFormatSupported(fmt)) {
        qWarning("QtAudio: default device doesn't support the requested format; "
                 "QtMultimedia will resample.");
    }
    sink_ = std::make_unique<QAudioSink>(out, fmt);
    // ~200 ms. The Qt6 Pulse / PipeWire backends call readData with chunks
    // sized to a fraction of the buffer; bigger buffer → fewer + larger
    // readData calls → fewer chances for the libresidfp resampler to
    // underrun and stitch zero-padded chunks together (audible as the
    // 'ticks' on transients the user reported).
    // 80 ms sink buffer. The 200 ms value used previously masked audio-thread
    // stalls but produced an audible playhead-vs-audio lag in the editor —
    // the display snapshot ring now compensates for whatever the sink holds
    // anyway, so a shorter buffer cuts both display delay (visible to the
    // user) and the underlying audio latency.
    sink_->setBufferSize(sampleRate * 80 / 1000 * 2);
    sink_->setVolume(1.0);
    device_ = new PullDevice(sampleRate, this);
    sink_->start(device_);
    qInfo() << "QtAudio: device=" << out.description()
            << "buffer(B)=" << sink_->bufferSize()
            << "state=" << sink_->state();
    if (sink_->state() == QAudio::StoppedState) {
        qWarning("QtAudio: sink failed to start: %d", (int)sink_->error());
        return false;
    }
    return true;
}

void QtAudio::stop() {
    if (sink_) sink_->stop();
    sink_.reset();
    device_ = nullptr;
}

// RAII fence: lock the audio mutex (waits for any in-flight readData to
// finish), then hard-stop the playroutine. The destructor releases the lock
// so the next fill resumes with whatever state the caller installed.
AudioFence::AudioFence() {
    auto *a = QtAudio::instance();
    if (a) {
        a->suspend();
        a->mutex().lock();
    }
    stopsong();
    songinit = PLAY_STOPPED;
}
AudioFence::~AudioFence() {
    auto *a = QtAudio::instance();
    if (a) {
        a->mutex().unlock();
        a->resume();
    }
}
