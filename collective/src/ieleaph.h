/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef SQMPACKETHANDLER_H
#define SQMPACKETHANDLER_H

// qt core libs
#include <QtCore/QObject>
#include <QtCore/QIODevice>
#include <QtCore/QVariant>
#include <QtCore/QtEndian>

// qt network libs
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

// include ansi libs
#include <string>

//
// PACKETLENGTHTYPE
//
// The datatype will read at first from datastream to determinate the content length
// NOTE: this value has to be the same on client and server, if not, the packet parser will not work properly
//
#define PACKETLENGTHTYPE quint32

//
// PROPERTYNAME
//
// The property key will be used to store the non finished packet in the QObject meta system of the sending IODevice
// NOTE: don't use key names which start with _q_, because this will be used by Qt itself
//
#define PROPERTYNAME_PACKET "SQMPacketHandler_packet"

//
// DataPacket
//
// The DataPacket will be generated by the packet parser as result after successfully parse process of an packet!
//
struct DataPacket
{
    QIODevice* ioPacketDevice;
    QByteArray* baRawPacketData;
    PACKETLENGTHTYPE intPacktLength;
};

class IEleaph : public QObject
{
    Q_OBJECT
    public:
        // device forget options for device addings
        enum DeviceForgetOptions {
            ForgetDeviceOnClose = 0,
            ForgetDeviceOnDestroy = 2,
            NeverForgetDevice = 3
        };

        // start tcp listening
        bool startTcpListening(quint16 port, QHostAddress address = QHostAddress::Any);

        // static datapacket send functions
        static void sendDataPacket(QIODevice* device, QByteArray *baDatatoSend);
        static void sendDataPacket(QIODevice* device, std::string strDatatoSend);

    public slots:
        void addDevice(QIODevice* device, IEleaph::DeviceForgetOptions forgetoptions = IEleaph::ForgetDeviceOnDestroy);
        void removeDevice(QIODevice *device = 0);

    protected:
        // protected con and decon for inhertance override
        // set max length by default to 20MB (20971520 Bytes)
        IEleaph(quint32 maxDataLength = 20971520, QObject *parent = 0);
        ~IEleaph();

    protected:
        // pure virtual methods
        virtual void deviceAdded(QIODevice* device);
        virtual void deviceRemoved(QIODevice* device);
        virtual void newDataPacketReceived(DataPacket *dataPacket) = 0;

    private slots:
        void newTcpHost();
        void dataHandler();

    private:
        // dynamic members
        quint32 intMaxDataLength;

        // members for tcpserver feature
        QTcpServer serverTcp;
};

#endif // SQMPACKETHANDLER_H