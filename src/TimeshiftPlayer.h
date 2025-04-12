#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>
#include <QTimer>

#include "bass.h"

class TimeshiftPlayer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString archiveRoot READ archiveRoot WRITE setArchiveRoot NOTIFY archiveRootChanged)
    Q_PROPERTY(QDateTime currentDateTime READ currentDateTime NOTIFY currentDateTimeChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY isPlayingChanged)
    Q_PROPERTY(QString currentMetadata READ currentMetadata NOTIFY currentMetadataChanged)
public:
    explicit TimeshiftPlayer(QObject *parent = nullptr);
    ~TimeshiftPlayer();
    
    QString archiveRoot() const;
    void setArchiveRoot(const QString &root);
    QDateTime currentDateTime() const;
    bool isPlaying() const;
    QString currentMetadata() const;
    
    Q_INVOKABLE void playAt(const QDateTime &dt);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void skipForward(int minutes = 1);
    Q_INVOKABLE void skipBackward(int minutes = 1);
    Q_INVOKABLE void jumpToLatestWeatherForecast();
    Q_INVOKABLE void jumpToLatestNews();

signals:
    void archiveRootChanged();
    void currentDateTimeChanged();
    void isPlayingChanged();
    void currentMetadataChanged();
    void playbackError(const QString &errorMessage);

private slots:
    void checkPlaybackStatus();
    void processMetadata();

private:
    HSTREAM m_stream;
    QString m_archiveRoot;
    QDateTime m_currentDateTime;
    bool m_isPlaying;
    QString m_currentMetadata;
    QTimer m_statusTimer;
    QTimer m_metadataTimer;
    
    QString filePathFromDateTime(const QDateTime &dt) const;
    bool loadAndPlayFile(const QString &path);
    void freeCurrentStream();
    void loadNextMinute();
    QString extractMetadataFromStream();
};
