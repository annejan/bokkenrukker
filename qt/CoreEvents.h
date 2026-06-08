#pragma once
#include <QObject>
#include <atomic>

// CoreEvents — the notification bridge that replaces per-frame UI polling.
//
// Playback state (transport on/off, the play row, the song order position)
// mutates in the PortAudio callback thread (PaAudio::paCallback ->
// playroutine()).
//
// SPLIT across the thread boundary, RT-safe:
//   * pump()    runs on the AUDIO thread, once per playroutine tick. It does
//               NOTHING but cheap int compares + lock-free atomic stores — no
//               heap allocation, no mutex, no Qt. On a state edge it bumps an
//               atomic generation counter. (Emitting a Qt signal directly from
//               here was the original mistake: a queued emit allocates a
//               QMetaCallEvent and locks the GUI event queue, which under
//               repaint contention stalls the realtime callback -> underruns ->
//               silence.)
//   * deliver() runs on the GUI thread, called from MainWindow's tick. It reads
//               the atomic generation counters (3 cheap loads) and, only when
//               one changed, emits the matching Qt signal — from the GUI thread,
//               so the connection is an ordinary same-thread emit.
//
// Net effect is still event-driven: the heavy work (repaints) happens only when
// something actually changed. The only "poll" left is 3 atomic loads per tick,
// which never touches the audio thread.
class CoreEvents : public QObject {
    Q_OBJECT
public:
    static CoreEvents *instance() { return self_; }
    explicit CoreEvents(QObject *parent = nullptr);
    ~CoreEvents() override;

    // AUDIO thread: detect state edges, bump atomic counters. Lock-free.
    void pump();

    // GUI thread: emit the queued-up edges as signals. Call once per UI tick.
    void deliver();

signals:
    void transportChanged(bool playing); // play started / stopped
    void rowChanged();                   // play row advanced on any channel
    void orderPosChanged();              // song order position advanced

private:
    static CoreEvents *self_;

    // Audio-thread-only diff state (no sharing — pump() is the sole accessor).
    bool     lastPlaying_  = false;
    unsigned lastRowSig_   = 0;
    unsigned lastOrderSig_ = 0;
    bool     primed_       = false;

    // Cross-thread hand-off: audio thread bumps the *Gen_ counters / stores
    // playing_; GUI thread reads them in deliver(). Lock-free.
    std::atomic<unsigned> transportGen_{0};
    std::atomic<unsigned> rowGen_{0};
    std::atomic<unsigned> orderGen_{0};
    std::atomic<bool>     playing_{false};

    // GUI-thread-only: last generation already emitted.
    unsigned seenTransport_ = 0;
    unsigned seenRow_       = 0;
    unsigned seenOrder_     = 0;
};
