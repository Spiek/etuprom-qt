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
#include "../../collective/proto/src/eleaphrpc.pb.h"

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
        struct ProtoRPCPacket
        {
            DataPacket *dataPacket;
            QString strProcedureName;
            qint32 intChannel;
            QMap<QString, QString> mapKeyValues;
        };

        // con / decon
        EleaphProtoRPC(QObject *parent = 0, quint32 maxDataLength = 20971520);

        // rpc funtions
        void registerRPCMethod(QString strMethod, QObject* receiver, const char *member, bool singleShot = false);
        void unregisterRPCMethod(QString strMethod, QObject* receiver = 0, const char *member = 0);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, QMap<QString, QString> mapKeyValues, qint32 channel = 0);

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
