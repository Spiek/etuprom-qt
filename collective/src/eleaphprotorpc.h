/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#ifndef ELEAPHPROTORPC_H
#define ELEAPHPROTORPC_H

// eleaph interface
#include "ieleaph.h"

// qt core libs
#include <QtCore/QMultiMap>
#include <QtCore/QMetaObject>

// google protobuf
#include "eleaphrpc.pb.h"

class EleaphProtoRPC : public IEleaph
{
    Q_OBJECT
    public:
        // structure definations
        struct Delegate
        {
            QObject* object;
            QByteArray method;
            bool singleShot;
        };

        // con / decon
        EleaphProtoRPC(QObject *parent = 0, quint32 maxDataLength = 20971520);

        //
        // RPC funtions
        //

        // register/unregister
        void registerRPCMethod(QString strMethod, QObject* receiver, const char *member, bool singleShot = false);
        void unregisterRPCMethod(QString strMethod, QObject* receiver = 0, const char *member = 0);

        // sending
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, std::string);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, char* data, int length);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, QByteArray data);

    protected:
        // interface implementation
        virtual void newDataPacketReceived(DataPacket *dataPacket);

    private:
        // rpc members
        QMultiMap<QString, EleaphProtoRPC::Delegate*> mapRPCFunctions;

        // helper methods
        QByteArray extractMethodName(const char* method);
};

#endif // ELEAPHPROTORPC_H
