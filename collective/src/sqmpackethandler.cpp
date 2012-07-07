/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "SQMPacketHandler"



//
// Con's and Decon
//

/*
 * SQMPacketHandler - construct the PacketHandler
 *                    NOTE: protected constructor for SINGELTON construction
 */
SQMPacketHandler::SQMPacketHandler(quint32 maxDataLength, QObject *parent) : QObject(parent)
{
    // save construct vars
    this->intMaxDataLength = maxDataLength;
}

/*
 * ~SQMPacketHandler - deconstructor
 */
SQMPacketHandler::~SQMPacketHandler()
{
    // cleanup
}



//
// SLOT section
//

/*
 * addDevice - add device for packet parsing
 */
void SQMPacketHandler::addDevice(QIODevice* device, bool forgetonclose)
{
    // connect to PacketHanderss
    this->connect(device, SIGNAL(readyRead()), this, SLOT(dataHandler()));

    // if user want that the Packethandler forget the device on close,
    // remove the device after device has closed
    if(forgetonclose) {
        this->connect(device, SIGNAL(aboutToClose()), this, SLOT(removeDevice()));
    }

    // if user want that the Packethandler don't forget the device on close,
    // remove the device after device was destroyed
    else {
        this->connect(device, SIGNAL(destroyed()), this, SLOT(removeDevice()));
    }

    // inform the world about the new connected device
    emit this->deviceUsageChanged(device, true);
}

/*
 * removeDevice - remove device from packet parsing
 */
void SQMPacketHandler::removeDevice(QIODevice *device)
{
    // aquire device by param or as sender, if not possible, exit
    QIODevice *ioPacketDevice = !device ? qobject_cast<QIODevice*>(this->sender()) : device;
    if(!ioPacketDevice) {
        return;
    }

    // remove all connected signals device --> this and this --> device
    ioPacketDevice->disconnect(this);
    this->disconnect(ioPacketDevice);

    // delete all used properties
    QVariant variantStoredPackage = ioPacketDevice->property(PROPERTYNAME_PACKET);
    DataPacket *packet = variantStoredPackage.type() == QVariant::Invalid ? (DataPacket*)0 : (DataPacket*)variantStoredPackage.value<void *>();
    if(packet) {
        delete packet;
    }

    // delete properties
    ioPacketDevice->setProperty(PROPERTYNAME_PACKET, QVariant(QVariant::Invalid));

    // inform the world about the removed device
    emit this->deviceUsageChanged(ioPacketDevice, false);
}

/*
 * dataHandler - packet parser logic, will called by every device on which data is available
 */
void SQMPacketHandler::dataHandler()
{
    // get the sending QIODevice and exit if it's not a valid
    QIODevice *ioPacketDevice = qobject_cast<QIODevice*>(this->sender());
    if(!ioPacketDevice) {
        return;
    }

    /// <Aquire Data Packet>

    // get the exesting data packet, or if it doesn't exist a 0 Pointer
    QVariant variantStoredPackage = ioPacketDevice->property(PROPERTYNAME_PACKET);
    DataPacket *packet = variantStoredPackage.type() == QVariant::Invalid ? (DataPacket*)0 : (DataPacket*)variantStoredPackage.value<void *>();

    // create new data packet if data packet doesn't exist
    if(!packet) {
        DataPacket *packetNew = new DataPacket;
        packet = packetNew;

        // initialize default values and add new packet to progress map
        packetNew->intPacktLength = 0;
        packetNew->baRawPacketData = 0;
        packetNew->ioPacketDevice = ioPacketDevice;
        ioPacketDevice->setProperty(PROPERTYNAME_PACKET, qVariantFromValue((void *) packetNew));
    }

    /// </Aquire Data Packet> <-- Packet was successfull aquired!
    /// <Read Header>

    // simplefy some header lengths
    PACKETLENGTHTYPE intAvailableDataLength = ioPacketDevice->bytesAvailable();

    // read header if it is not present, yet
    if(!packet->intPacktLength) {
        // if not enough data available to read complete header, exit here and wait for more data!
        if(intAvailableDataLength < sizeof(PACKETLENGTHTYPE)) {
            return;
        }

        // otherwise, enough data is present to read the complete header, so do it :-)
        // read content length with the help of Qt's Endian method qFromBigEndian
        QByteArray baPacketLength = ioPacketDevice->read(sizeof(PACKETLENGTHTYPE));
        PACKETLENGTHTYPE* ptrPacketLength = (PACKETLENGTHTYPE*)baPacketLength.constData();
        packet->intPacktLength = qFromBigEndian<PACKETLENGTHTYPE>(*ptrPacketLength);

        // security check:
        // if content length is greater than the allowed intMaxDataLength, close the device immediately
        if(packet->intPacktLength > this->intMaxDataLength) {
            return ioPacketDevice->close();
        }
    }

    /// </Read Header> <-- Header read complete!
    /// <Read Content>

    // if not enough data is present to read complete content, exit here and wait for more data!
    if(intAvailableDataLength < packet->intPacktLength) {
        return;
    }

    // read the complete content of packet
    packet->baRawPacketData = new QByteArray(ioPacketDevice->read(packet->intPacktLength));

    // and send packet as signal out in the world ("the world" has the task to delete it!)
    emit this->newPacketReceived(packet);

    // at this point the entire packet was read and sent:
    // now we delete all used properties
    ioPacketDevice->setProperty(PROPERTYNAME_PACKET, QVariant(QVariant::Invalid));

    /// </Read Content> <-- Content read complete!

    // if there is still data on the socket call myself recrusivly again
    if(intAvailableDataLength > packet->intPacktLength) {
        return this->dataHandler();
    }
}

/*
 * newTcpHost - add new connected tcp host to packet parser
 *              and make sure that the socket will deleted properly
 */
void SQMPacketHandler::newTcpHost()
{
    QTcpSocket *socket = this->serverTcp.nextPendingConnection();

    // delete device on disconnect
    this->connect(socket, SIGNAL(disconnected()), this, SLOT(removeDevice()));

    // add the device to packet parser and remove the device if it's destroyed
    this->addDevice(socket, false);
}




//
// Public section
//

/*
 * startTcpListening - start listenening on given adress and port
 */
bool SQMPacketHandler::startTcpListening(quint16 port, QHostAddress address)
{
    // handle new connected tcp clients
    this->connect(&this->serverTcp, SIGNAL(newConnection()), this, SLOT(newTcpHost()));
    return this->serverTcp.listen(address, port);
}




//
// Static Helper section
//

/*
 * sendDataPacket - send given data to given IODevice including packet length
 */
void SQMPacketHandler::sendDataPacket(DataPacket *dpSrc, std::string strDatatoSend)
{
   QByteArray baData(strDatatoSend.c_str(), strDatatoSend.length());
   return SQMPacketHandler::sendDataPacket(dpSrc->ioPacketDevice, &baData);
}

/*
 * sendDataPacket - send given data to given IODevice including packet length
 */
void SQMPacketHandler::sendDataPacket(DataPacket *dpSrc, QByteArray *baDatatoSend)
{
   return SQMPacketHandler::sendDataPacket(dpSrc->ioPacketDevice, baDatatoSend);
}

/*
 * sendDataPacket - send given data to given IODevice including packet length
 */
void SQMPacketHandler::sendDataPacket(QIODevice *device, std::string strDatatoSend)
{
    QByteArray baData(strDatatoSend.c_str(), strDatatoSend.length());
    return SQMPacketHandler::sendDataPacket(device, &baData);
}

/*
 * sendDataPacket - send given data to given IODevice including packet length
 */
void SQMPacketHandler::sendDataPacket(QIODevice *device, QByteArray *baDatatoSend)
{
    // create content length with the help of Qt's Endian method qToBigEndian
    PACKETLENGTHTYPE intDataLength = baDatatoSend->length();
    intDataLength = qToBigEndian<PACKETLENGTHTYPE>(intDataLength);

    // send the content-length and data
    device->write((char*)&intDataLength, sizeof(PACKETLENGTHTYPE));
    device->write(*baDatatoSend);
}



//
// SINGELTON section
//

// static variable declaration
SQMPacketHandler* SQMPacketHandler::sqmPacketHandler = 0;

// SINGELTON Constructor
void SQMPacketHandler::create(QObject *object, quint32 maxDataLength)
{
    // only create instance if we haven't allready one
    if(!SQMPacketHandler::sqmPacketHandler) {
        SQMPacketHandler::sqmPacketHandler = new SQMPacketHandler(maxDataLength, object);
    }
}

// SINGELTON instance getter method
SQMPacketHandler* SQMPacketHandler::getInstance()
{
    return SQMPacketHandler::sqmPacketHandler;
}
