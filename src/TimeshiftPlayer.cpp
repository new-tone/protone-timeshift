#include "TimeshiftPlayer.h"
#include <QDebug>
#include <QDir>
#include <QString>
#include <QDateTime>

TimeshiftPlayer::TimeshiftPlayer(QObject *parent)
    : QObject(parent), m_archiveRoot("/Users/gk/radio-929/archeo")
{
    // Initialize BASS (for Mac, use the default device, sample rate settings)
    if (!BASS_Init(-1, 44100, 0, 0, nullptr)) {
        qWarning() << "BASS_Init error!";
    }
}

QString TimeshiftPlayer::archiveRoot() const {
    return m_archiveRoot;
}

void TimeshiftPlayer::setArchiveRoot(const QString &root) {
    if (m_archiveRoot != root) {
        m_archiveRoot = root;
        emit archiveRootChanged();
    }
}

QString TimeshiftPlayer::filePathFromDateTime(const QDateTime &dt) const {
    // Construct path: archiveRoot/year/month/day/hour/year-month-day_hour-minute-00.mp3
    QString year = dt.toString("yyyy");
    QString month = dt.toString("MM");
    QString day = dt.toString("dd");
    QString hour = dt.toString("HH");
    QString basePath = QString("%1/%2/%3/%4/%5")
                .arg(m_archiveRoot, year, month, day, hour);
    QString filename = dt.toString("yyyy-MM-dd_HH-mm-00") + ".mp3";
    return QDir::cleanPath(basePath + "/" + filename);
}

void TimeshiftPlayer::playAt(const QDateTime &dt) {
    QString path = filePathFromDateTime(dt);
    qDebug() << "Playing file:" << path;
    // Create stream from file
    m_stream = BASS_StreamCreateFile(FALSE, path.toUtf8().constData(), 0, 0, 0);
    if (!m_stream) {
        qWarning() << "Failed to create BASS stream for" << path;
        return;
    }
    // Play the stream (restart from beginning)
    if (!BASS_ChannelPlay(m_stream, FALSE)) {
        qWarning() << "Failed to play stream!";
    }
}
