#include "Log.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QTextStream>
#include <QtGlobal>

#include <cstdio>

namespace {
QFile  *g_file = nullptr;
QMutex  g_mtx;
QString g_path;

const char *typeTag(QtMsgType t) {
    switch (t) {
    case QtDebugMsg:    return "DBG";
    case QtInfoMsg:     return "INF";
    case QtWarningMsg:  return "WRN";
    case QtCriticalMsg: return "ERR";
    case QtFatalMsg:    return "FTL";
    }
    return "???";
}

void handler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    QString ts = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString line = QString("[%1] %2 %3").arg(ts, typeTag(type), msg);
    if (ctx.file && ctx.line) {
        line += QString(" (%1:%2)").arg(ctx.file).arg(ctx.line);
    }
    {
        QMutexLocker lk(&g_mtx);
        if (g_file) {
            g_file->write(line.toLocal8Bit());
            g_file->write("\n");
            g_file->flush();
        }
    }
    // Also echo to stderr — keeps `--rpc | tee` and console runs informative.
    std::fprintf(stderr, "%s\n", line.toLocal8Bit().constData());
    if (type == QtFatalMsg) std::abort();
}
} // namespace

namespace Log {

QString init() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (dir.isEmpty()) dir = QDir::tempPath();
    QDir().mkpath(dir);
    g_path = dir + "/goattrk2-qt.log";

    g_file = new QFile(g_path);
    if (!g_file->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        std::fprintf(stderr, "Log: cannot open %s\n", qPrintable(g_path));
        delete g_file;
        g_file = nullptr;
    }
    qInstallMessageHandler(handler);
    qInfo("goattrk2-qt log opened at %s", qPrintable(g_path));
    return g_path;
}

QString path() { return g_path; }

} // namespace Log
