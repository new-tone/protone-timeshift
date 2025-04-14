#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QQuickImageProvider>
#include <QJsonObject>
#include <QFont>

class IconProvider : public QQuickImageProvider
{
public:
    explicit IconProvider(const QString &familyPath, const QString &codesPath);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
private:
    QMap<QString, QString> codepoints;
    QFont font;
    int hexStringToOct(QString hexInput);
};

#endif // ICONPROVIDER_H
