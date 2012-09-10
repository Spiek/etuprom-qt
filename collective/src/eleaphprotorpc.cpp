/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "eleaphprotorpc.h"

EleaphProtoRPC::EleaphProtoRPC(QObject *parent, quint32 maxDataLength) : IEleaph(maxDataLength, parent)
{
    // register the ProtoPacket
    qRegisterMetaType<ProtoRPCPacket>("ProtoRPCPacket");
}

//
// Public Access functions
//
/*
 * sendRPCDataPacket - send an RPC DataPacket to given Device
 */
void EleaphProtoRPC::registerRPCMethod(QString strMethod, QObject *receiver, const char *member)
{
    // normalize method and call delegate
    QByteArray methodNormalized = this->extractMethodName(member);

    // create delegate which points on the receiver
    EleaphProtoRPC::Delegate delegate;
    delegate.object = receiver;
    delegate.method = methodNormalized;

    // ... and save the informations
    this->mapRPCFunctions.insertMulti(strMethod, delegate);
}

/*
 * sendRPCDataPacket - send an RPC DataPacket to given Device
 */
void EleaphProtoRPC::sendRPCDataPacket(QIODevice *device, QString strProcedureName, QMap<QString, QString> mapKeyValues, qint32 channel)
{
    // create Protobuf RPC-Packet
    EleaphRPCProtocol::Packet *packetProto = new EleaphRPCProtocol::Packet;
    packetProto->set_procedurename(strProcedureName.toStdString());
    packetProto->set_channel(channel);

    // add key values
    foreach(QString strKey, mapKeyValues.keys()) {
        EleaphRPCProtocol::DataField *dataField = packetProto->add_data();
        dataField->set_key(strKey.toStdString());
        dataField->set_value(mapKeyValues.value(strKey).toStdString());
    }

    // send the RPC-Packet
    this->sendDataPacket(device, packetProto->SerializeAsString());
}


//
// Interface implementations
//

/*
 * newDataPacketReceived - parse the new received dataPacket and forward it to registered Delegate(s)
 */
void EleaphProtoRPC::newDataPacketReceived(DataPacket *dataPacket)
{
    // deserialize packet
    EleaphRPCProtocol::Packet *packetProto = new EleaphRPCProtocol::Packet;
    packetProto->ParseFromArray(dataPacket->baRawPacketData->constData(), dataPacket->baRawPacketData->size());

    // simplefy some packet values
    QString strMethodName = QString::fromStdString(packetProto->procedurename());

    // if given procedure name of the packet is not registered, then cleanup and exit
    if(!this->mapRPCFunctions.contains(strMethodName)) {
        delete packetProto;
        return;
    }

    // create ProtoPacket with all needed informations
    ProtoRPCPacket *protoPacket = new ProtoRPCPacket;
    protoPacket->dataPacket = dataPacket;
    protoPacket->strProcedureName = strMethodName;
    protoPacket->intChannel = packetProto->channel();

    // set key-value - values
    for(int i = 0; i < packetProto->data_size(); i++) {
        EleaphRPCProtocol::DataField *field = packetProto->mutable_data(i);
        protoPacket->mapKeyValues.insert(QString::fromStdString(field->key()), QString::fromStdString(field->value()));
    }

    // ... loop all delegates which are registered for strMethodName, and invoke them one by one
    foreach(EleaphProtoRPC::Delegate delegate, this->mapRPCFunctions.values(strMethodName)) {
        // simplefy the delegate
        QObject* object = delegate.object;
        QByteArray method = delegate.method;

        // call delegate
        QMetaObject::invokeMethod(object, method.constData(), Q_ARG(ProtoRPCPacket*, protoPacket));
    }

    // after work is done, delete the packert
    delete dataPacket;
}

/*
 * extractMethodName - normalize SIGNAL and SLOT functionname to normal methodname
 */
QByteArray EleaphProtoRPC::extractMethodName(const char *member)
{
    // code from qt source (4.8.2)
    // src: qtimer.cpp
    // line: 354
    const char* bracketPosition = strchr(member, '(');
    if (!bracketPosition || !(member[0] >= '0' && member[0] <= '3')) {
        qWarning("EleaphProtoRPC::extractMethodName: Invalid slot specification");
        return QByteArray();
    }
    return QByteArray(member+1, bracketPosition - 1 - member); // extract method name
}
