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

    static QtAudio *instance() { return self_; }

private:
    class PullDevice;
    std::unique_ptr<QAudioSink> sink_;
    PullDevice *device_ = nullptr;
    int sampleRate_ = 0;
    QMutex mtx_;
    static QtAudio *self_;
};

// RAII guard: suspend the sink + take the audio mutex for the duration of a
// non-thread-safe C-globals rewrite (loadsong / clearsong / sid_init reset).
class AudioFence {
public:
    AudioFence();
    ~AudioFence();
};
