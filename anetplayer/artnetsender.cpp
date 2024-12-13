#include "artnetsender.h"


ArtNetSender::ArtNetSender(QObject *parent)
    : QObject(parent)
{}

bool ArtNetSender::sendTime(const QString &time)
{
    QByteArray dmxData = convertTimeToByteArray(time);
    if (prepareArtNetPacket(dmxData))
    {
        return true;
    }
    return false;
}

bool ArtNetSender::prepareArtNetPacket(const QByteArray &data)
{
    if (targetAddress.isNull() && !(targetPort > 0)) {
        return false;
    }

    QByteArray packet;
    packet.append("Art-Net\0", 8);               // Prefix "Art-Net"
    packet.append(static_cast<char>(0x00));      // OpCode for ArtNet Timecode (0x9700, little-endian)
    packet.append(static_cast<char>(0x97));      // OpCode
    packet.append(static_cast<char>(0x00));      // Protocol version
    packet.append(static_cast<char>(0x0E));      // Protocol version
    packet.append(static_cast<char>(0x00));      // Filler (ignored by receiver)
    packet.append(static_cast<char>(0x00));      // StreamId
    packet.append(data); // Add the actual data

    udpSocket.writeDatagram(packet, targetAddress, targetPort); // Send
    return true;
}

QByteArray ArtNetSender::convertTimeToByteArray(const QString &time)
{
    QByteArray dmxData(5, 0);

    QStringList timeParts = time.split(':');
    if (timeParts.size() != 5) {
        emit sendMsg("Invalid time format. Expected format hh:mm:ss:ff:fps.");
        return dmxData;
    }

    bool ok = false;
    uint8_t hours = timeParts[0].toInt(&ok);
    uint8_t minutes = timeParts[1].toInt(&ok);
    uint8_t seconds = timeParts[2].toInt(&ok);
    uint8_t frames = timeParts[3].toInt(&ok);
    uint8_t fps = timeParts[4].toInt(&ok);
    uint8_t type = 0x00;
        // 0 = Film (24fps)
        // 1 = EBU (25fps)
        // 2 = DF (29.97fps)
        // 3 = SMPTE (30fps)
    switch (fps) {
    case 24:
        type = 0x00;
        break;
    case 25:
        type = 0x01;
        break;
    case 29:
        type = 0x02;
        break;
    case 30:
        type = 0x03;
        break;
    default:
        break;
    }

    if (!ok)
    {
        emit sendMsg("Invalid time values.");
        return dmxData;
    }

    // byte array time data
    dmxData[3] = static_cast<char>(hours);
    dmxData[2] = static_cast<char>(minutes);
    dmxData[1] = static_cast<char>(seconds);
    dmxData[0] = static_cast<char>(frames);
    dmxData[4] = static_cast<char>(type);

    return dmxData;
}

bool ArtNetSender::setNetworkInterface(const QString &interfaceName)
{
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
    {
        if (interface.humanReadableName() == interfaceName && interface.flags().testFlag(QNetworkInterface::IsUp))
        {
            foreach (const QNetworkAddressEntry &entry, interface.addressEntries())
            {
                if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    if (udpSocket.state() != QAbstractSocket::UnconnectedState) {
                        udpSocket.close();
                    }
                    udpSocket.bind(entry.ip(), targetPort, QUdpSocket::ShareAddress);
                    emit sendMsg("Bound to interface: " + interfaceName + "  with IP: " + entry.ip().toString());
                    return true;
                }
            }
        }
    }
    emit sendMsg("Failed to bind to interface:" + interfaceName);
    return false;
}

QStringList ArtNetSender::getAvailableInterfaces()
{
    QStringList interfaces;
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces())
    {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) && !interface.addressEntries().isEmpty()) {
            interfaces.append(interface.humanReadableName());
        }
    }
    return interfaces;
}

void ArtNetSender::setTargetIP(const QString &ipAddress)
{
    targetAddress = QHostAddress(ipAddress);
}

void ArtNetSender::setTargetPort(quint16 port)
{
    targetPort = port;
}
