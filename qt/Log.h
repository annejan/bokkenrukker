#pragma once
#include <QString>

namespace Log {
    // Open log file (truncate). Install Qt message handler so all
    // qDebug/qInfo/qWarning/qCritical land in both stderr and the file.
    // Returns the absolute path of the log file.
    QString init();
    QString path();
}
