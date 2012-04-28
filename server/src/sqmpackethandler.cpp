/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "sqmpackethandler.h"

//
// Con's and Decon
//

SQMPacketHandler::SQMPacketHandler(QObject *parent) : QObject(parent)
{
    // create tmp Data Stream for protocol parsing, later
    this->dataStreamTmp = new QDataStream;
}

SQMPacketHandler::~SQMPacketHandler()
{
    // cleanup
    delete this->dataStreamTmp;
}


//
// SLOT handlings!
//

/*
 * newDevice - add device for packet parsing
 */
void SQMPacketHandler::newDevice(QIODevice* device)
{
    // append device to device list and connect socket with packet parser
    this->lstPeers.append(device);
    this->connect(device, SIGNAL(readyRead()), this, SLOT(dataHandler()));
}

/*
 * newDevice - remove device from packet parsing
 */
void SQMPacketHandler::disconnectedDevice(QIODevice *device)
{
    // remove device from device list and disconnect socket from packet parser
    this->lstPeers.removeOne(device);
    device->disconnect(this);
    this->disconnect(device);
}

/*
 * dataHandler - protocol parser logic, will called by every device on which data is available
 */
void SQMPacketHandler::dataHandler()
{
    // get the sending QIODevice
    QIODevice *ioPacketDevice = qobject_cast<QIODevice*>(this->sender());

    /// <Aquire Data Packet>

    // get a pointer to a Packet by searching in map for an existing Datapacket, or create a new one
    DataPacket *packet = 0;
    if(!this->mapPacketsInProgress.contains(ioPacketDevice)) {
        DataPacket *packetNew = new DataPacket;
        packet = packetNew;

        // initialize default values and add new packet to progress map
        packetNew->intPacktLength = 0;
        packetNew->baRawPacketData = 0;
        packetNew->ioPacketDevice = ioPacketDevice;
        this->mapPacketsInProgress.insert(ioPacketDevice, packetNew);
    } else {
        packet = this->mapPacketsInProgress.value(ioPacketDevice);
    }

    /// </Aquire Data Packet> <-- Packet was successfull aquired!
    /// <Read Header>

    // simplefy some header lengths
    PACKETLENGTHTYPE intAvailableDataLength = ioPacketDevice->bytesAvailable();
    PACKETLENGTHTYPE intHeaderPacketLength = sizeof(PACKETLENGTHTYPE);

    // read header if it is not present, yet
    if(!packet->intPacktLength) {
        // if not enough data available to read complete header, exit here and wait for more data!
        if(intAvailableDataLength < intHeaderPacketLength) {
            return;
        }

        // otherwise, enough data is present to read the complete header, so do it :-)
        else {
            // read content length with the help of Qt's powerful Datastream serializer
            this->dataStreamTmp->setDevice(ioPacketDevice);
            (*this->dataStreamTmp) >> packet->intPacktLength;
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

    // at this point the entire packet was read:
    // now we delete the packet from the "packet-in-progress"-list
    this->mapPacketsInProgress.remove(ioPacketDevice);

    // and send packet as signal out in the world ("the world" has the task to delete it!)
    emit this->newPacketReceived(packet);

    /// </Read Content> <-- Header content complete!

    // if there is still data on the socket call myself recrusivly again
    if(intAvailableDataLength > packet->intPacktLength) {
        return this->dataHandler();
    }
}
