#ifndef CONTACTMANAGER_H
#define CONTACTMANAGER_H

#include <QObject>

// own libs
#include "collective/proto/packettypes.h"
#include "protocol.pb.h"
#include "global.h"

// sub module dependings
#include "usermanager.h"

class Contactmanager : public QObject
{
    Q_OBJECT
    public:
        // con and decon
        Contactmanager(EleaphProtoRPC* eleaphRPC, Usermanager* managerUser, QObject *parent = 0);

    private:
		EleaphProtoRPC* eleaphRPC;
		Usermanager* managerUser;

	private slots:
        void handleContactList(EleaphRpcPacket dataPacket);
        void handleContactChange(Usermanager::SharedSession session);
};

#endif // CONTACTMANAGER_H
