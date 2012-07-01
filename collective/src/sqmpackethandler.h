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

class SQMPacketHandler : public QObject
{
    Q_OBJECT
    signals:
        void newPacketReceived(DataPacket *packet);

    public slots:
        void newDevice(QIODevice* device);
        void disconnectedDevice(QIODevice *device = 0);
        void dataHandler();

    public:
        // singelton static functions
        static void create(QObject *object = 0, quint32 maxDataLength = 20971520);
        static SQMPacketHandler* getInstance();

        // static helper functions
        static void sendDataPacket(DataPacket* dpSrc, QByteArray *baDatatoSend);
        static void sendDataPacket(QIODevice* device, QByteArray *baDatatoSend);

    protected:
        // protected con and decon so that no one (except the static create method) is able to construct an object!+
        // set max length by default to 20MB
        SQMPacketHandler(quint32 maxDataLength = 20971520, QObject *parent = 0);
        ~SQMPacketHandler();

    private:
        // static memeber for singelton
        static SQMPacketHandler *sqmPacketHandler;

        // dynamic members
        quint32 intMaxDataLength;
};

#endif // SQMPACKETHANDLER_H