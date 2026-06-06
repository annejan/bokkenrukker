#include "QtAudio.h"

#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QIODevice>
#include <QDebug>
#include <cstring>

extern "C" {
#include "gcommon.h"
void playroutine(void);
int  sid_fillbuffer(short *ptr, int samples);
extern unsigned multiplier;
extern unsigned ntsc;
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
        // Cache the frame rate calculation outside the inner loop. ntsc /
        // multiplier are read-only during a buffer fill; recomputing per
        // chunk was a non-trivial portion of the per-buffer cost.
        const int frame = (ntsc ? 60 : 50) * (multiplier ? multiplier : 1);
        const int samplesPerTick = sampleRate_ / frame;
        short *out = reinterpret_cast<short*>(data);
        const qint64 frames = maxlen / 2;
        qint64 produced = 0;
        while (produced < frames) {
            if (sampleAccum_ <= 0) {
                playroutine();
                sampleAccum_ += samplesPerTick;
            }
            const qint64 chunk = std::min<qint64>(sampleAccum_, frames - produced);
            const int got = sid_fillbuffer(out + produced, (int)chunk);
            if (got <= 0) {
                // libresidfp held back samples. Don't memset to zero — that
                // is exactly the tick / click the user is hearing. Hold the
                // last sample instead so the waveform stays continuous; the
                // next playroutine tick will refill genuine data.
                short hold = (produced > 0) ? out[produced - 1] : 0;
                for (qint64 i = 0; i < chunk; i++) out[produced + i] = hold;
                produced += chunk;
                sampleAccum_ -= chunk;
                continue;
            }
            produced += got;
            sampleAccum_ -= got;
        }
        return produced * 2;
    }
    qint64 writeData(const char *, qint64) override { return -1; }
    qint64 bytesAvailable() const override {
        return std::numeric_limits<qint64>::max();
    }
private:
    int  sampleRate_;
    int  sampleAccum_ = 0;
};

QtAudio::QtAudio(QObject *parent) : QObject(parent) {}
QtAudio::~QtAudio() { stop(); }

bool QtAudio::start(int sampleRate) {
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
    sink_->setBufferSize(sampleRate * 200 / 1000 * 2);
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
