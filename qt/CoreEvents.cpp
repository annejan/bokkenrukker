#include "CoreEvents.h"

extern "C" {
#include "gcommon.h"   // MAX_CHN
#include "gplay.h"     // CHN, chn[], isplaying()
}
extern "C" int song_channels;

CoreEvents *CoreEvents::self_ = nullptr;

CoreEvents::CoreEvents(QObject *parent) : QObject(parent) { self_ = this; }
CoreEvents::~CoreEvents() { if (self_ == this) self_ = nullptr; }

void CoreEvents::pump() {
    int nc = song_channels;
    if (nc <= 0 || nc > MAX_CHN) nc = MAX_CHN;

    const bool playing = isplaying() != 0;

    // Cheap order-independent signatures over the per-channel play position.
    // Any movement of the play row / order pointer on any active channel
    // changes the signature and is reported as a single edge.
    unsigned rowSig = 0, orderSig = 0;
    for (int c = 0; c < nc; c++) {
        rowSig   = rowSig   * 131u + chn[c].pattptr;
        orderSig = orderSig * 131u + chn[c].songptr;
    }

    // First call just records the baseline so we don't fire a spurious edge.
    if (!primed_) {
        lastPlaying_  = playing;
        lastRowSig_   = rowSig;
        lastOrderSig_ = orderSig;
        primed_       = true;
        return;
    }

    // On an edge, bump the matching atomic generation counter. NO Qt, no
    // alloc, no lock — this runs on the realtime audio thread. The GUI thread
    // turns these into signals in deliver().
    if (playing != lastPlaying_) {
        lastPlaying_ = playing;
        playing_.store(playing, std::memory_order_relaxed);
        transportGen_.fetch_add(1, std::memory_order_release);
    }
    if (rowSig != lastRowSig_) {
        lastRowSig_ = rowSig;
        rowGen_.fetch_add(1, std::memory_order_release);
    }
    if (orderSig != lastOrderSig_) {
        lastOrderSig_ = orderSig;
        orderGen_.fetch_add(1, std::memory_order_release);
    }
}

void CoreEvents::deliver() {
    // GUI thread: read the audio thread's edge counters and emit the signals
    // here, where CoreEvents lives, so each emit is an ordinary same-thread
    // call. Cheap: 3 atomic loads, and the body only runs on an actual change.
    const unsigned t = transportGen_.load(std::memory_order_acquire);
    if (t != seenTransport_) {
        seenTransport_ = t;
        emit transportChanged(playing_.load(std::memory_order_relaxed));
    }
    const unsigned r = rowGen_.load(std::memory_order_acquire);
    if (r != seenRow_) {
        seenRow_ = r;
        emit rowChanged();
    }
    const unsigned o = orderGen_.load(std::memory_order_acquire);
    if (o != seenOrder_) {
        seenOrder_ = o;
        emit orderPosChanged();
    }
}
