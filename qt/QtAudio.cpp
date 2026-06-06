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
        return produced * 2;
    }
    qint64 writeData(const char *, qint64) override { return -1; }
    qint64 bytesAvailable() const override {
        return std::numeric_limits<qint64>::max();
    }
private:
    int     sampleRate_;
    double  sampleAccumF_ = 0.0;
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
