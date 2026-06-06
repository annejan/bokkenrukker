#pragma once
#include <QUndoStack>
#include <QUndoCommand>
#include <QByteArray>
#include <QString>

// Whole-song snapshot. The pattern / order / instrument / table data lives in
// fixed-size C globals, so we just memcpy them into a QByteArray on push and
// restore on undo/redo. About 150 KB per snapshot — cheap enough for the
// stack-size cap we set in MainWindow.

QByteArray captureSongSnapshot();
void restoreSongSnapshot(const QByteArray &blob);

class SongSnapshotCommand : public QUndoCommand {
public:
    // before = state before the user-visible edit, captured by the caller.
    // after  = current state, captured at construction.
    SongSnapshotCommand(QByteArray before, const QString &text);
    void undo() override;
    void redo() override;
    int id() const override { return 1; }
    bool mergeWith(const QUndoCommand *other) override;
private:
    QByteArray before_;
    QByteArray after_;
    bool firstRedo_ = true;
};

// Helper that returns a captured snapshot the caller can hand to
// SongSnapshotCommand later (after the edit).
inline QByteArray beginSongEdit() { return captureSongSnapshot(); }
