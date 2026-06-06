#include "UndoStack.h"
#include <cstring>

extern "C" {
#include "gcommon.h"
extern unsigned char pattern[MAX_PATT][MAX_PATTROWS*4+4];
extern int pattlen[MAX_PATT];
extern unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
extern int songlen[MAX_SONGS][MAX_CHN];
extern INSTR instr[MAX_INSTR];
extern unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
extern unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
extern char songname[MAX_STR];
extern char authorname[MAX_STR];
extern char copyrightname[MAX_STR];
}

namespace {
struct Snapshot {
    unsigned char pattern[MAX_PATT][MAX_PATTROWS*4+4];
    int pattlen[MAX_PATT];
    unsigned char songorder[MAX_SONGS][MAX_CHN][MAX_SONGLEN+2];
    int songlen[MAX_SONGS][MAX_CHN];
    INSTR instr[MAX_INSTR];
    unsigned char ltable[MAX_TABLES][MAX_TABLELEN];
    unsigned char rtable[MAX_TABLES][MAX_TABLELEN];
    char songname[MAX_STR];
    char authorname[MAX_STR];
    char copyrightname[MAX_STR];
};
}

QByteArray captureSongSnapshot() {
    QByteArray buf(sizeof(Snapshot), Qt::Uninitialized);
    Snapshot *s = reinterpret_cast<Snapshot*>(buf.data());
    std::memcpy(s->pattern,        ::pattern,        sizeof s->pattern);
    std::memcpy(s->pattlen,        ::pattlen,        sizeof s->pattlen);
    std::memcpy(s->songorder,      ::songorder,      sizeof s->songorder);
    std::memcpy(s->songlen,        ::songlen,        sizeof s->songlen);
    std::memcpy(s->instr,          ::instr,          sizeof s->instr);
    std::memcpy(s->ltable,         ::ltable,         sizeof s->ltable);
    std::memcpy(s->rtable,         ::rtable,         sizeof s->rtable);
    std::memcpy(s->songname,       ::songname,       sizeof s->songname);
    std::memcpy(s->authorname,     ::authorname,     sizeof s->authorname);
    std::memcpy(s->copyrightname,  ::copyrightname,  sizeof s->copyrightname);
    return buf;
}

void restoreSongSnapshot(const QByteArray &blob) {
    if (blob.size() != sizeof(Snapshot)) return;
    const Snapshot *s = reinterpret_cast<const Snapshot*>(blob.constData());
    std::memcpy(::pattern,       s->pattern,       sizeof ::pattern);
    std::memcpy(::pattlen,       s->pattlen,       sizeof ::pattlen);
    std::memcpy(::songorder,     s->songorder,     sizeof ::songorder);
    std::memcpy(::songlen,       s->songlen,       sizeof ::songlen);
    std::memcpy(::instr,         s->instr,         sizeof ::instr);
    std::memcpy(::ltable,        s->ltable,        sizeof ::ltable);
    std::memcpy(::rtable,        s->rtable,        sizeof ::rtable);
    std::memcpy(::songname,      s->songname,      sizeof ::songname);
    std::memcpy(::authorname,    s->authorname,    sizeof ::authorname);
    std::memcpy(::copyrightname, s->copyrightname, sizeof ::copyrightname);
}

SongSnapshotCommand::SongSnapshotCommand(QByteArray before, const QString &text)
    : QUndoCommand(text), before_(std::move(before)) {
    after_ = captureSongSnapshot();
}

void SongSnapshotCommand::undo() { restoreSongSnapshot(before_); }

void SongSnapshotCommand::redo() {
    if (firstRedo_) { firstRedo_ = false; return; }
    restoreSongSnapshot(after_);
}

bool SongSnapshotCommand::mergeWith(const QUndoCommand *other) {
    const auto *o = static_cast<const SongSnapshotCommand*>(other);
    // Coalesce runs of edits with the same description (e.g. successive note
    // entries within the pattern editor) so a single Ctrl+Z reverts a phrase.
    if (o->text() != text()) return false;
    after_ = o->after_;
    return true;
}
