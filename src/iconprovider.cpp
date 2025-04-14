#include "iconprovider.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QPainter>
#include <QFontMetrics>
#include <QFontDatabase>

IconProvider::IconProvider(const QString &familyPath, const QString &codesPath)
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    int id = QFontDatabase::addApplicationFont(familyPath);
    QString fontFamily = QFontDatabase::applicationFontFamilies(id).at(0);
    font = QFont(fontFamily);
    QFile file(codesPath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList tokens = line.split(" ");
            QString key = tokens[0];
            QString value = tokens[1];
            codepoints.insert(key, value);
        }
        file.close();
    } else {
        qWarning() << "Cannot open icon codes file" << codesPath;
        qWarning() << file.errorString();
    }
}

QImage IconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    int width = 48;
    int height = 48;

    if (requestedSize.width() > 0)
        width = requestedSize.width();

    if (requestedSize.height() > 0)
        height = requestedSize.height();

    if (size)
        *size = QSize(width, height);

    font.setPixelSize(width < height ? width : height);

    QImage image(width, height, QImage::Format_RGBA8888);
    image.fill(QColor(Qt::transparent));

    QString iconChar("?");

    QString value(codepoints.value(id).toUpper());
    if (!value.isEmpty())
    {
        QByteArray hex =  QByteArray::fromHex(QString("FEFF").append(value).toUtf8());
        iconChar = QString::fromUtf16((char16_t*)hex.data());
        iconChar.resize(1);
    }

    QFontMetrics fontMetrics(font);
    double widthRatio = double(width) / fontMetrics.boundingRect(iconChar).width();
    if (widthRatio < 1.0)
        font.setPixelSize(int(font.pixelSize() * widthRatio));

    QPainter painter(&image);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, width, height), Qt::AlignCenter, iconChar);

    iconChar.clear();

    return image;
}

int IconProvider::hexStringToOct(QString hexInput)
{
    if (hexInput.size() != 4)
        return 0;

    char hexDecNum[4];
    for(int i = 0; i < 4; i++)
        hexDecNum[i] = static_cast<char>(hexInput.at(i).toLatin1());

    int i, len=0, decNum=0, octNum=0;
    len = strlen(hexDecNum);
    len--;
    for(i=0; hexDecNum[i]!='\0'; i++, len--)
    {
        if(hexDecNum[i]>='0' && hexDecNum[i]<='9')
            decNum = decNum + (hexDecNum[i]-48)*pow(16,len);
        else if(hexDecNum[i]>='A' && hexDecNum[i]<='F')
            decNum = decNum + (hexDecNum[i]-55)*pow(16,len);
        else if(hexDecNum[i]>='a' && hexDecNum[i]<='f')
            decNum = decNum + (hexDecNum[i]-87)*pow(16,len);
        else
            return 0;
    }
    i=1;
    while(decNum!=0)
    {
        octNum = octNum + (decNum%8)*i;
        i = i*10;
        decNum = decNum/8;
    }
    return octNum;
}
