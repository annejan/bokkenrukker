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

    if (playing != lastPlaying_) {
        lastPlaying_ = playing;
        emit transportChanged(playing);
    }
    if (rowSig != lastRowSig_) {
        lastRowSig_ = rowSig;
        emit rowChanged();
    }
    if (orderSig != lastOrderSig_) {
        lastOrderSig_ = orderSig;
        emit orderPosChanged();
    }
}
