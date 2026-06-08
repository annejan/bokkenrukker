#pragma once
#include <QObject>

// CoreEvents — the notification bridge that replaces per-frame UI polling.
//
// Playback state (transport on/off, the play row, the song order position)
// mutates in the PortAudio callback thread (PaAudio::paCallback ->
// playroutine()). CoreEvents::pump() is called from THAT thread once per
// playroutine tick. It compares the current core state against the last-seen
// values (members touched only by the audio thread, so no locking needed) and,
// on an actual edge, emits a Qt signal.
//
// CoreEvents lives on the GUI thread, so a default (Auto) connection from the
// audio thread is delivered as a QueuedConnection — the slot runs safely on the
// GUI thread and no QWidget is ever touched from the audio thread. Edges are
// low-rate (row ~10-25 Hz, transport/order rare), so the per-edge queued post is
// cheap. This turns "every view re-reads globals every frame" into "views update
// only when something actually changed".
//
// RT note: emitting a queued signal from the audio callback does a small
// allocation + lock under the hood. It happens only on edges (not per sample),
// which is fine for this desktop, ~10 ms-buffer PortAudio setup. If audio
// glitches ever trace back here, the fallback is to bump atomic generation
// counters in pump() and drain them from the GUI tick instead.
class CoreEvents : public QObject {
    Q_OBJECT
public:
    static CoreEvents *instance() { return self_; }
    explicit CoreEvents(QObject *parent = nullptr);
    ~CoreEvents() override;

    // Called from the audio thread after each playroutine() tick. Detects
    // state edges and emits the matching signal. No-op-cheap when nothing
    // changed or nothing is connected.
    void pump();

signals:
    void transportChanged(bool playing); // play started / stopped
    void rowChanged();                   // play row advanced on any channel
    void orderPosChanged();              // song order position advanced

private:
    static CoreEvents *self_;
    // Last-seen state — only the audio thread (pump) reads/writes these.
    bool     lastPlaying_  = false;
    unsigned lastRowSig_   = 0;
    unsigned lastOrderSig_ = 0;
    bool     primed_       = false;
};
