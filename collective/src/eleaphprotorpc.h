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
#include <QtCore/QEventLoop>
#include <QtCore/QSharedPointer>

// forward declarations
class EleaphProcessEventHandler;

struct ElaphRpcPacketData : EleaphPacketData
{
    QString strMethodName;
};

typedef QSharedPointer<ElaphRpcPacketData> EleaphRpcPacket;


class EleaphProcessEvent
{
    public:
        enum class Type {
            Invalid = 0,
            Before = 1,
            After = 2
        };
        inline EleaphProcessEvent(QObject* receiver = 0, EleaphProcessEvent::Type type = EleaphProcessEvent::Type::Invalid) { this->receiver = receiver; this->type = type; }
        EleaphProcessEvent::Type type;
        QObject* receiver;

   friend class EleaphProcessEventHandler;
};

class EleaphProcessEvent_Before : EleaphProcessEvent {
    public:
        EleaphProcessEvent_Before(QObject* receiver) : EleaphProcessEvent(receiver, EleaphProcessEvent::Type::Before) { }
};

class EleaphProcessEvent_After : EleaphProcessEvent {
    public:
        EleaphProcessEvent_After(QObject* receiver) : EleaphProcessEvent(receiver, EleaphProcessEvent::Type::After) { }
};

class EleaphProtoRPC : public IEleaph
{
    Q_OBJECT
    signals:
        void sigDeviceAdded(QIODevice* device);
        void sigDeviceRemoved(QIODevice* device);

    public:
        // structure definations
        struct Delegate
        {
            QObject* object;
            QByteArray method;
            EleaphProcessEventHandler* additionalEventHandler;
            bool singleShot;
        };

        // con / decon
        EleaphProtoRPC(QObject *parent = 0, quint32 maxDataLength = 20971520);

        //
        // RPC funtions
        //

        // register/unregister
        void registerRPCMethod(QString strMethod, QObject* receiver, const char *member, bool singleShot = false, EleaphProcessEvent event0 = EleaphProcessEvent(), EleaphProcessEvent event1 = EleaphProcessEvent(), EleaphProcessEvent event2 = EleaphProcessEvent(), EleaphProcessEvent event3 = EleaphProcessEvent(), EleaphProcessEvent event4 = EleaphProcessEvent(), EleaphProcessEvent event5 = EleaphProcessEvent(), EleaphProcessEvent event6 = EleaphProcessEvent(), EleaphProcessEvent event7 = EleaphProcessEvent());
        void unregisterRPCMethod(QString strMethod, QObject* receiver = 0, const char *member = 0);
        void unregisterRPCMethod(QObject* receiver, const char *member = 0);

        // sending
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, std::string);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, char* data, int length);
        void sendRPCDataPacket(QIODevice *device, QString strProcedureName, QByteArray data = QByteArray());

        // asyncron wait
        EleaphRpcPacket waitAsyncForPacket(QString strMethod);

    protected:
        // interface implementation
        virtual void newDataPacketReceived(EleaphPacketData *dataPacket);
        virtual void deviceAdded(QIODevice* device);
        virtual void deviceRemoved(QIODevice* device);

    private slots:
        void unregisterRPCObject();

    private:
        // rpc members
        QMultiMap<QString, EleaphProtoRPC::Delegate*> mapRPCFunctions;

        // helper methods
        QByteArray extractMethodName(const char* method);
};


class EleaphProcessEventHandler : public QObject
{
    Q_OBJECT
    public:
        EleaphProcessEventHandler(QList<EleaphProcessEvent> events) { this->lstEvents = events; }

    public slots:
        void processPacket(EleaphProtoRPC::Delegate* delegate, EleaphRpcPacket packet)
        {
            // process all "beforePacketProcess" Events, if one event result with false, abort packet process
            if(!this->processEvent(delegate, packet, EleaphProcessEvent::Type::Before)) {
                return;
            }

            // simplefy the delegate
            QObject* object = delegate->object;
            QByteArray method = delegate->method;

            // call delegate syncronly (because we are in the receiver thread)
            QMetaObject::invokeMethod(object, method.constData(), Qt::DirectConnection, Q_ARG(EleaphRpcPacket, packet));

            // process all "afterPacketProcess" Events
            this->processEvent(delegate, packet, EleaphProcessEvent::Type::After);
        }

    private:
        QList<EleaphProcessEvent> lstEvents;

        bool processEvent(EleaphProtoRPC::Delegate* delegate, EleaphRpcPacket packet, EleaphProcessEvent::Type type)
        {
            // so let us loop all events
            foreach(EleaphProcessEvent event, this->lstEvents) {
                // only process needed events and valid ones
                if(event.type == EleaphProcessEvent::Type::Invalid || event.type != type) {
                    continue;
                }

                // simplefy objects
                QObject* object = event.receiver;
                const QMetaObject* metaObject = object->metaObject();

                QString strEventMethodName = type == EleaphProcessEvent::Type::Before ? "beforePacketProcessed" : "afterPacketProcessed";
                if(metaObject->indexOfMethod(strEventMethodName.toStdString().c_str()) != -1) {
                    bool continueProcess = true;
                    QMetaObject::invokeMethod(object, strEventMethodName.toStdString().c_str(), Qt::DirectConnection, Q_ARG(EleaphProtoRPC::Delegate*, delegate), Q_ARG(EleaphRpcPacket, packet), Q_ARG(bool*, &continueProcess));

                    // don't continue process the packet!
                    if(!continueProcess) {
                        return false;
                    }
                }
            }

            // process event
            return true;
        }
};

class ElepahAsyncPacketWaiter : public QObject
{
    Q_OBJECT
    signals:
        void packetReady();

    public:
        EleaphRpcPacket receivedDataPacket;
        ElepahAsyncPacketWaiter(EleaphProtoRPC* eleaphProto, QString strMethod)
        {
            eleaphProto->registerRPCMethod(strMethod, this, SLOT(packetReceived(EleaphRpcPacket)), true);
        }

    public slots:
        void packetReceived(EleaphRpcPacket dataPacket)
        {
            this->receivedDataPacket = dataPacket;
            emit packetReady();
        }

};

#endif // ELEAPHPROTORPC_H
