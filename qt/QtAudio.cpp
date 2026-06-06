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
    qint64 readData(char *data, qint64 maxlen) override {
        const int frame = (ntsc ? 60 : 50) * (multiplier ? multiplier : 1);
        const int samplesPerTick = sampleRate_ / frame;
        short *out = reinterpret_cast<short*>(data);
        qint64 frames = maxlen / 2;        // 16-bit mono
        qint64 produced = 0;
        while (produced < frames) {
            if (sampleAccum_ <= 0) {
                playroutine();
                sampleAccum_ += samplesPerTick;
            }
            int chunk = std::min<qint64>(sampleAccum_, frames - produced);
            int got = sid_fillbuffer(out + produced, chunk);
            if (got <= 0) {
                std::memset(out + produced, 0, (frames - produced) * 2);
                produced = frames;
                break;
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
    sink_->setBufferSize(sampleRate / 25 * 2);  // ~40 ms latency
    device_ = new PullDevice(sampleRate, this);
    sink_->start(device_);
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
