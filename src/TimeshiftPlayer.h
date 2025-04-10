#pragma once

#include <QObject>
#include <QDateTime>
#include <QString>

#include "bass.h"

class TimeshiftPlayer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString archiveRoot READ archiveRoot WRITE setArchiveRoot NOTIFY archiveRootChanged)
public:
    explicit TimeshiftPlayer(QObject *parent = nullptr);
    
    QString archiveRoot() const;
    void setArchiveRoot(const QString &root);
    
    Q_INVOKABLE void playAt(const QDateTime &dt);

signals:
    void archiveRootChanged();

private:
    HSTREAM m_stream;
    QString m_archiveRoot;
    QString filePathFromDateTime(const QDateTime &dt) const;
};
