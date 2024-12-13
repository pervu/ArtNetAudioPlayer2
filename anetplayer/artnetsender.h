#ifndef ARTNETSENDER_H
#define ARTNETSENDER_H

#include <QObject>
#include <QUdpSocket>
#include <QString>
#include <QHostAddress>
#include <QNetworkInterface>

class ArtNetSender : public QObject
{
    Q_OBJECT

public:
    explicit ArtNetSender(QObject *parent = nullptr);
    bool sendTime(const QString &time);
    bool setNetworkInterface(const QString &interfaceName);
    static QStringList getAvailableInterfaces();
    void setTargetIP(const QString &ipAddress);
    void setTargetPort(quint16 port = 6454);

private:
    bool prepareArtNetPacket(const QByteArray &data);
    QByteArray convertTimeToByteArray(const QString &time);

    QUdpSocket udpSocket;
    QHostAddress targetAddress;
    quint16 targetPort = 0;

signals:
    void sendMsg(const QString &msg);

};

#endif // ARTNETSENDER_H
