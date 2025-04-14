#include "timeshiftplayer.h"

#include <QDebug>
#include <QDir>
#include <QString>
#include <QFile>
#include <QDateTime>

// Define callback for BASS to notify when playback ends
void CALLBACK EndPlaybackCallback(HSYNC handle, DWORD channel, DWORD data, void *user);

TimeshiftPlayer::TimeshiftPlayer(QObject *parent)
    : QObject(parent),
      m_archiveRoot("/Users/gk/radio-929/archeo"),
      m_stream(0),
      m_isPlaying(false),
      m_currentMetadata("")
{
    // Initialize BASS (for Mac, use the default device, sample rate settings)
    if (!BASS_Init(-1, 44100, 0, 0, nullptr)) {
        qWarning() << "BASS_Init error: " << BASS_ErrorGetCode();
    }
    
    // Set up the timer for checking playback status
    connect(&m_statusTimer, &QTimer::timeout, this, &TimeshiftPlayer::checkPlaybackStatus);
    m_statusTimer.setInterval(500);  // Check every 500ms
    
    // Set up the timer for metadata processing
    connect(&m_metadataTimer, &QTimer::timeout, this, &TimeshiftPlayer::processMetadata);
    m_metadataTimer.setInterval(2000);  // Check for metadata every 2 seconds
}

TimeshiftPlayer::~TimeshiftPlayer() {
    // Clean up BASS resources
    freeCurrentStream();
    BASS_Free();
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

QDateTime TimeshiftPlayer::currentDateTime() const {
    return m_currentDateTime;
}

bool TimeshiftPlayer::isPlaying() const {
    return m_isPlaying;
}

QString TimeshiftPlayer::currentMetadata() const {
    return m_currentMetadata;
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
    stop();
    
    m_currentDateTime = dt;
    QString path = filePathFromDateTime(dt);
    
    if (loadAndPlayFile(path)) {
        m_isPlaying = true;
        emit isPlayingChanged();
        emit currentDateTimeChanged();
        
        // Start the timers for playback monitoring
        m_statusTimer.start();
        m_metadataTimer.start();
    }
}

void TimeshiftPlayer::pause() {
    if (m_stream && m_isPlaying) {
        BASS_ChannelPause(m_stream);
        m_isPlaying = false;
        emit isPlayingChanged();
        m_statusTimer.stop();
        m_metadataTimer.stop();
    }
}

void TimeshiftPlayer::resume() {
    if (m_stream && !m_isPlaying) {
        BASS_ChannelPlay(m_stream, FALSE);
        m_isPlaying = true;
        emit isPlayingChanged();
        m_statusTimer.start();
        m_metadataTimer.start();
    }
}

void TimeshiftPlayer::stop() {
    if (m_stream) {
        freeCurrentStream();
        m_isPlaying = false;
        emit isPlayingChanged();
        m_statusTimer.stop();
        m_metadataTimer.stop();
    }
}

void TimeshiftPlayer::skipForward(int minutes) {
    if (minutes <= 0) return;
    
    QDateTime newTime = m_currentDateTime.addSecs(minutes * 60);
    playAt(newTime);
}

void TimeshiftPlayer::skipBackward(int minutes) {
    if (minutes <= 0) return;
    
    QDateTime newTime = m_currentDateTime.addSecs(-minutes * 60);
    playAt(newTime);
}

void TimeshiftPlayer::jumpToLatestWeatherForecast() {
    // This is a placeholder that would need implementation based on metadata
    // or some other mechanism to identify weather forecasts in the archive
    qDebug() << "Jump to latest weather forecast - Not implemented yet";
    emit playbackError("Weather forecast detection not implemented yet");
}

void TimeshiftPlayer::jumpToLatestNews() {
    // This is a placeholder that would need implementation based on metadata
    // or some other mechanism to identify news segments in the archive
    qDebug() << "Jump to latest news - Not implemented yet";
    emit playbackError("News detection not implemented yet");
}

void TimeshiftPlayer::checkPlaybackStatus() {
    if (m_stream && m_isPlaying) {
        // Check if playback has ended
        QWORD position = BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE);
        QWORD length = BASS_ChannelGetLength(m_stream, BASS_POS_BYTE);
        
        // If we're near the end (last 100ms), prepare to load the next minute file
        double positionInSeconds = BASS_ChannelBytes2Seconds(m_stream, position);
        double lengthInSeconds = BASS_ChannelBytes2Seconds(m_stream, length);
        
        if (lengthInSeconds - positionInSeconds < 0.1) {
            loadNextMinute();
        }
    }
}

void TimeshiftPlayer::processMetadata() {
    if (m_stream && m_isPlaying) {
        QString metadata = extractMetadataFromStream();
        if (!metadata.isEmpty() && metadata != m_currentMetadata) {
            m_currentMetadata = metadata;
            emit currentMetadataChanged();
        }
    }
}

bool TimeshiftPlayer::loadAndPlayFile(const QString &path) {
    QFile file(path);
    if (!file.exists()) {
        qWarning() << "File does not exist:" << path;
        emit playbackError("File not found: " + path);
        return false;
    }
    
    // Create stream from file
    HSTREAM newStream = BASS_StreamCreateFile(FALSE, path.toUtf8().constData(), 0, 0, BASS_STREAM_AUTOFREE);
    if (!newStream) {
        qWarning() << "Failed to create BASS stream for" << path << "Error:" << BASS_ErrorGetCode();
        emit playbackError("Failed to create audio stream");
        return false;
    }
    
    // Store the new stream handle
    m_stream = newStream;
    
    // Play the stream
    if (!BASS_ChannelPlay(m_stream, FALSE)) {
        qWarning() << "Failed to play stream! Error:" << BASS_ErrorGetCode();
        emit playbackError("Failed to play audio");
        return false;
    }
    
    qDebug() << "Now playing:" << path;
    return true;
}

void TimeshiftPlayer::freeCurrentStream() {
    if (m_stream) {
        BASS_ChannelStop(m_stream);
        BASS_StreamFree(m_stream);
        m_stream = 0;
    }
}

void TimeshiftPlayer::loadNextMinute() {
    // Calculate the next minute
    QDateTime nextMinute = m_currentDateTime.addSecs(60);
    QString nextFilePath = filePathFromDateTime(nextMinute);
    
    // Check if the file exists before attempting to load it
    QFile nextFile(nextFilePath);
    if (nextFile.exists()) {
        // Store current playback position for smooth transition (if needed)
        double oldPos = 0;
        if (m_stream) {
            oldPos = BASS_ChannelBytes2Seconds(m_stream, BASS_ChannelGetPosition(m_stream, BASS_POS_BYTE));
            freeCurrentStream();
        }
        
        // Update current time and load the new file
        m_currentDateTime = nextMinute;
        if (loadAndPlayFile(nextFilePath)) {
            emit currentDateTimeChanged();
            qDebug() << "Loaded next minute file:" << nextFilePath;
        }
    } else {
        qWarning() << "Next minute file does not exist:" << nextFilePath;
        // Stop playback if no next file
        stop();
    }
}

QString TimeshiftPlayer::extractMetadataFromStream() {
    if (!m_stream) return QString();
    
    // ICY (Shoutcast) metadata handling
    //const char *id3 = BASS_ChannelGetTags(m_stream, BASS_TAG_ID3);
    //if (id3) {
    //    // Extract ID3 tag information
    //    QString artist = QString::fromLatin1(id3->artist, sizeof(id3->artist)).trimmed();
    //    QString title = QString::fromLatin1(id3->title, sizeof(id3->title)).trimmed();
    //
    //    if (!artist.isEmpty() && !title.isEmpty()) {
    //        return artist + " - " + title;
    //    }
    //}
    
    // Try to get ICY (SHOUTcast) metadata
    const char *icy = BASS_ChannelGetTags(m_stream, BASS_TAG_ICY);
    if (icy) {
        while (*icy) {
            if (!strncmp(icy, "ICY-NAME:", 9)) { // station name
                return QString::fromLatin1(icy + 9).trimmed();
            }
            icy += strlen(icy) + 1;
        }
    }
    
    // Try to get HTTP headers (includes ICY metadata)
    const char *http = BASS_ChannelGetTags(m_stream, BASS_TAG_HTTP);
    if (http) {
        while (*http) {
            if (!strncmp(http, "icy-name:", 9)) { // station name
                return QString::fromLatin1(http + 9).trimmed();
            }
            http += strlen(http) + 1;
        }
    }
    
    // Try to get meta tags (StreamTitle='artist - title')
    const char *meta = BASS_ChannelGetTags(m_stream, BASS_TAG_META);
    if (meta) {
        QString metaStr = QString::fromLatin1(meta);
        int titleStart = metaStr.indexOf("StreamTitle='");
        if (titleStart >= 0) {
            titleStart += 13;
            int titleEnd = metaStr.indexOf("';", titleStart);
            if (titleEnd > titleStart) {
                return metaStr.mid(titleStart, titleEnd - titleStart);
            }
        }
    }
    
    return QString();
}
