#ifndef ELEAPH_H
#define ELEAPH_H

// eleaph interface
#include "ieleaph.h"

class Eleaph : public IEleaph
{
    Q_OBJECT
    signals:
         void newPacketReceived(DataPacket *packet);
         void deviceUsageChanged(QIODevice* device, bool usage);

    public:
        Eleaph(QObject *parent = 0, quint32 maxDataLength = 20971520);

    protected:
        // interface implementation
        virtual void newDataPacketReceived(DataPacket *dataPacket);
        virtual void deviceAdded(QIODevice* device);
        virtual void deviceRemoved(QIODevice* device);
};

#endif // ELEAPH_H
