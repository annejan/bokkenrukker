#pragma once
#include <QObject>
#include <QIODevice>
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

private:
    class PullDevice;
    std::unique_ptr<QAudioSink> sink_;
    PullDevice *device_ = nullptr;
};
