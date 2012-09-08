#include "eleaph.h"

Eleaph::Eleaph(QObject *parent, quint32 maxDataLength) : IEleaph(maxDataLength, parent)
{

}


//
// Interface implementations
//

/*
 * deviceAdded - simply send the device via a SIGNAL out in the world!
 */
void Eleaph::deviceAdded(QIODevice *device)
{
    emit this->deviceUsageChanged(device, true);
}

/*
 * newDataPacketReceived - simply send the dataPacket via a SIGNAL out in the world!
 */
void Eleaph::newDataPacketReceived(DataPacket *dataPacket)
{
    emit newPacketReceived(dataPacket);
}

/*
 * deviceAdded - simply send the device via a SIGNAL out in the world!
 */
void Eleaph::deviceRemoved(QIODevice *device)
{
    emit this->deviceUsageChanged(device, false);
}
