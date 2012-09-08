/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "eleaphprotorpc.h"

EleaphProtoRPC::EleaphProtoRPC(QObject *parent, quint32 maxDataLength) : IEleaph(maxDataLength, parent)
{

}

//
// Public Access functions
//
void EleaphProtoRPC::registerRPCMethod(QString strMethod, QObject *receiver, const char *member)
{
    // create delegate which points on the receiver
    EleaphProtoRPC::Delegate delegate;
    delegate.object = receiver;
    delegate.method = member;

    // ... and save the informations
    this->mapRPCFunctions.insertMulti(strMethod, delegate);
}

//
// Interface implementations
//

/*
 * newDataPacketReceived - handle the procedurecall
 */
void EleaphProtoRPC::newDataPacketReceived(DataPacket *dataPacket)
{
    // deserialize packet
    EleaphRPCProtocol::Packet *packetProto = new EleaphRPCProtocol::Packet;
    packetProto->ParseFromString(QString(*dataPacket->baRawPacketData).toStdString());

    // simplefy some packet values
    QString strMethodName = QString::fromStdString(packetProto->procedurename());

    // if given procedure name of the packet is not registered, then cleanup and exit
    if(!this->mapRPCFunctions.contains(strMethodName)) {
        delete packetProto;
        return;
    }

    // ... if procedure for packet was found, loop all registered Delegates of the function and call one by one
    foreach(QString strProcedureName, this->mapRPCFunctions.keys()) {
        // skip function if function name is not the packet procedure name
        if(strProcedureName != strMethodName) {
            continue;
        }

        // simplefy the function values
        EleaphProtoRPC::Delegate delegate = this->mapRPCFunctions.value(strProcedureName);
        QObject* object = delegate.object;
        const char* method = delegate.method;

        // ...function found, so call it
        QMetaObject::invokeMethod(object, method, Q_ARG(google::protobuf::Message, *packetProto));
    }
}
