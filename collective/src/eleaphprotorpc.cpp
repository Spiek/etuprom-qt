/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

#include "eleaphprotorpc.h"

EleaphProtoRPC::EleaphProtoRPC(QObject *parent, quint32 maxDataLength) : IEleaph(maxDataLength, parent)
{
    // register the ProtoPacket
    qRegisterMetaType<EleaphRpcPacket>("EleaphRpcPacket");
}

//
// Public Access functions
//
/*
 * registerRPCMethod - register RPC Method for Async DataPacket handling
 */
void EleaphProtoRPC::registerRPCMethod(QString strMethod, QObject *receiver, const char *member, bool singleShot,
                                       EleaphRpcPacketMetaEvent event0,
                                       EleaphRpcPacketMetaEvent event1,
                                       EleaphRpcPacketMetaEvent event2,
                                       EleaphRpcPacketMetaEvent event3,
                                       EleaphRpcPacketMetaEvent event4,
                                       EleaphRpcPacketMetaEvent event5,
                                       EleaphRpcPacketMetaEvent event6,
                                       EleaphRpcPacketMetaEvent event7)

{
    // move all events to list
    QList<EleaphRpcPacketMetaEvent> events;
    if(event0.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event0);
    if(event1.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event1);
    if(event2.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event2);
    if(event3.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event3);
    if(event4.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event4);
    if(event5.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event5);
    if(event6.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event6);
    if(event7.type != EleaphRpcPacketMetaEvent::Type::Invalid) events.append(event7);

    // event thread check
    // every event have to be in the same thread as receiver object thread, if not we cannot call Events Syncronly, so remove event!
    for(int i = 0;i < events.length(); i++) {
        EleaphRpcPacketMetaEvent event = events.at(i);
        if(event.receiver->thread() != receiver->thread()) {
            qWarning("[%s %s::%s line:%i] Event-Receiver-Object of EleaphRpcPacketMetaEvent is not in the receivers Thread, \"%s\" event of \"%s\" method will not be processed!",
                     __FILE__,
                     typeid(*this).name(),
                     __func__ ,
                     __LINE__,
                     event.type == EleaphRpcPacketMetaEvent::Type::Before ? "before" : "after",
                     qPrintable(strMethod)
                     );
            events.removeAt(i);
        }
    }

    // normalize method
    QByteArray methodNormalized = this->extractMethodName(member);

    // create delegate which points on the receiver
    EleaphProtoRPC::Delegate* delegate = new EleaphProtoRPC::Delegate;
    delegate->object = receiver;
    delegate->method = methodNormalized;
    delegate->singleShot = singleShot;
    delegate->additionalEventHandler = new EleaphRpcPacketHandler(events);
    delegate->additionalEventHandler->moveToThread(receiver->thread());

    // if receiver was destroyed, remove it's rpc methods
    this->connect(receiver, SIGNAL(destroyed()), this, SLOT(unregisterRPCObject()));

    // ... and save the informations
    this->mapRPCFunctions.insertMulti(strMethod, delegate);
}

void EleaphProtoRPC::unregisterRPCMethod(QString strMethod, QObject *receiver, const char *member)
{
    // if no receiver was set, remove all registered procedures for given RPC-function-name
    if(!receiver) {
        this->mapRPCFunctions.remove(strMethod);
    }

    // otherwise we have to loop all registered procedures to determinate the right procedures for deletion
    else {
        // normalize method
        QByteArray methodNormalized = (member) ? this->extractMethodName(member) : QByteArray();

        // loop all registered procedures for given RPC-function-name
        foreach(Delegate *delegate, this->mapRPCFunctions.values(strMethod)) {
            // - if member was set, remove only RPC methods which match on receiver and on the member
            // - if no member was set, remove only RPC methods which match only on the receiver
            if((!member || delegate->method == methodNormalized) && delegate->object == receiver) {
                this->mapRPCFunctions.remove(strMethod, delegate);
                delete delegate;
            }
        }
    }
}

void EleaphProtoRPC::unregisterRPCMethod(QObject *receiver, const char *member)
{
    // normalize method
    QByteArray methodNormalized = (member) ? this->extractMethodName(member) : QByteArray();

    // loop all registered rpc methods
    foreach(QString strMethod, this->mapRPCFunctions.keys()) {
        // loop all registered delegates for rpc method
        foreach(Delegate *delegate, this->mapRPCFunctions.values(strMethod)) {
            // - if member was set, remove only RPC methods which match on receiver and on the member
            // - if no member was set, remove only RPC methods which match only on the receiver
            if(delegate->object == receiver && (!member || delegate->method == methodNormalized)) {
                this->mapRPCFunctions.remove(strMethod, delegate);
                delete delegate;
            }
       }
    }
}


/*
 * sendRPCDataPacket - OVERLOADED: send an RPC DataPacket to given Device
 */
void EleaphProtoRPC::sendRPCDataPacket(QIODevice *device, QString strProcedureName, char *data, int length)
{
    return this->sendRPCDataPacket(device, strProcedureName, QByteArray(data, length));
}

/*
 * sendRPCDataPacket - OVERLOADED: send an RPC DataPacket to given Device
 */
void EleaphProtoRPC::sendRPCDataPacket(QIODevice *device, QString strProcedureName, std::string data)
{
    return this->sendRPCDataPacket(device, strProcedureName, QByteArray(data.c_str(), data.length()));
}

/*
 * sendRPCDataPacket - send an RPC DataPacket to given Device
 */
void EleaphProtoRPC::sendRPCDataPacket(QIODevice *device, QString strProcedureName, QByteArray data)
{
    // create content length with the help of Qt's Endian method qToBigEndian
    qint16 intDataLength = strProcedureName.length();
    intDataLength = qToBigEndian<qint16>(intDataLength);

    // prepend the content-length and method name to the data
    data.prepend(strProcedureName.toLatin1());
    data.prepend((char*)&intDataLength, sizeof(qint16));

    // send the RPC-Packet
    this->sendDataPacket(device, &data);
}

EleaphRpcPacket EleaphProtoRPC::waitAsyncForPacket(QString strMethod)
{
    ElepahAsyncPacketWaiter packetWaiter(this, strMethod);

    // wait for packet with the help of the event loop
    QEventLoop eventLoop;
    eventLoop.connect(&packetWaiter, SIGNAL(packetReady()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    // return packet
    return packetWaiter.receivedDataPacket;
}


//
// Interface implementations
//

/*
 * newDataPacketReceived - parse the new received dataPacket and forward it to registered Delegate(s)
 */
void EleaphProtoRPC::newDataPacketReceived(EleaphPacket *dataPacket)
{
    // extract rpc method name from packet with the help of Qt's Endian method qFromBigEndian
    qint16* ptrPacketLength = (qint16*)dataPacket->baRawPacketData->data();
    qint16 lenData = qFromBigEndian<qint16>(*ptrPacketLength);
    QString strMethodName = QString(dataPacket->baRawPacketData->mid(sizeof(qint16), lenData));

    // if given procedure name of the packet is not registered, then cleanup and exit
    if(!this->mapRPCFunctions.contains(strMethodName)) {
        return;
    }

    // remove EleaphRPCProtocol::Packet from data
    int intRPCPacketLength = sizeof(qint16) + lenData;
    dataPacket->baRawPacketData->remove(0, intRPCPacketLength);
    dataPacket->intPacktLength -= intRPCPacketLength;

    // constuct rpc datapacket (and move all data from DataPacket to EleaphRPCDataPacket)
    ElaphRpcPacketData* rpcDataPacket = (ElaphRpcPacketData*)dataPacket->move(new ElaphRpcPacketData, true);
    EleaphRpcPacket watcher(rpcDataPacket);

    // set EleaphRPCDataPacket data
    rpcDataPacket->strMethodName = strMethodName;

    // ... loop all delegates which are registered for strMethodName, and invoke them one by one
    foreach(EleaphProtoRPC::Delegate *delegate, this->mapRPCFunctions.values(strMethodName)) {
        // let the EleaphAdditionalEventHandler do the rest (int the event loop of the receivers thread!)
        emit delegate->additionalEventHandler->processPacket(delegate, watcher);
    }
}

void EleaphProtoRPC::deviceAdded(QIODevice *device)
{
    emit this->sigDeviceAdded(device);
}

void EleaphProtoRPC::deviceRemoved(QIODevice *device)
{
    emit this->sigDeviceRemoved(device);
}


//
// private slots
//

void EleaphProtoRPC::unregisterRPCObject()
{
    QObject *objToUnregister = this->sender();

    // unregister all rpc methods which match on object
    return this->unregisterRPCMethod(objToUnregister);
}



//
// Helper Methods
//

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
        return QByteArray();
    }
    return QByteArray(member+1, bracketPosition - 1 - member); // extract method name
}
