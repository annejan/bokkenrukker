#pragma once
#include <QObject>
#include <QIODevice>
#include <QMutex>
#include <memory>

class QAudioSink;

// Push-pull audio sink built on QtMultimedia's QAudioSink. Replaces the
// SDL 1.2 + bme_snd audio path for the Qt build — Qt routes its audio
// output through PipeWire / PulseAudio / ALSA / WASAPI / CoreAudio without
// us having to know which is active on the host.
class QtAudio : public QObject {
    Q_OBJECT
public:
    explicit QtAudio(QObject *parent = nullptr);
    ~QtAudio() override;

    bool start(int sampleRate);
    void stop();

    // Suspend the audio thread without tearing down the sink — used to
    // bracket non-thread-safe operations (loadsong / clearsong) on the C
    // globals that the audio thread reads each fill.
    void suspend();
    void resume();

    // PullDevice::readData locks this mutex for the entire fill, so a
    // MainWindow load/clear path can grab it to fence off the audio thread
    // synchronously — QAudioSink::suspend() alone is async and lets the
    // in-flight readData race the C-globals rewrite.
    QMutex &mutex() { return mtx_; }

    // Display-side snapshot of chn[] taken at each playroutine tick during
    // a fill. The UI consults this through currentSnapshot() instead of
    // reading chn[c] directly, so the visible playhead / blink reflects
    // what is being heard NOW (~200 ms behind the fill thanks to the sink
    // buffer) instead of what is being filled into the sink.
    struct PlaySnapshot {
        int  pattptr[6];   // MAX_CHN
        int  songptr[6];
        int  instr  [6];
    };
    PlaySnapshot currentSnapshot();

    static QtAudio *instance() { return self_; }

    // Push a snapshot of chn[] tagged with the sample cursor at the time
    // the playroutine ran. Called by PullDevice after each tick.
    void pushSnapshot(qint64 sampleAt);

private:
    class PullDevice;
    std::unique_ptr<QAudioSink> sink_;
    PullDevice *device_ = nullptr;
    int sampleRate_ = 0;
    QMutex mtx_;

    // Ring buffer of recent ticks. Sized for ~1 s at 50 Hz so the UI can
    // always find the one matching processedUSecs() even with a fat sink
    // buffer or follow-play overshoot.
    static constexpr int kRingN = 128;
    struct Tick { qint64 sampleAt; PlaySnapshot snap; };
    Tick    ring_[kRingN];
    int     ringHead_ = 0;   // next slot to write
    int     ringFill_ = 0;

    static QtAudio *self_;
};

// RAII guard: suspend the sink + take the audio mutex for the duration of a
// non-thread-safe C-globals rewrite (loadsong / clearsong / sid_init reset).
class AudioFence {
public:
    AudioFence();
    ~AudioFence();
};
