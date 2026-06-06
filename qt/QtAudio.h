#pragma once
#include <QObject>
#include <QIODevice>
#include <memory>

class QAudioSink;

// Push-pull audio sink built on QtMultimedia's QAudioSink. Replaces the
// SDL 1.2 + bme_snd audio path for the Qt build — Qt routes its audio
// output through PipeWire / PulseAudio / ALSA / WASAPI / CoreAudio without
// us having to know which is active on the host.
class AudioOut : public QObject {
    Q_OBJECT
public:
    explicit AudioOut(QObject *parent = nullptr);
    ~AudioOut() override;

    bool start(int sampleRate);
    void stop();

    // Suspend the audio thread without tearing down the sink — used to
    // bracket non-thread-safe operations (loadsong / clearsong) on the C
    // globals that the audio thread reads each fill.
    void suspend();
    void resume();

    static AudioOut *instance() { return self_; }

private:
    class PullDevice;
    std::unique_ptr<QAudioSink> sink_;
    PullDevice *device_ = nullptr;
    int sampleRate_ = 0;
    static AudioOut *self_;
};
