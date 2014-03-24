/*
 * This work is licensed under the Creative Commons Attribution 3.0 Unported License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by/3.0/
 * or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.
 */

// Qt (core)
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

// Qt (sql)
#include <QtSql/QSqlDatabase>

// own libs
#include "global.h"
#include "packetprocessor.h"

// collective own libs
#include "EleaphRpc"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // set some program values
    a.setApplicationName("SQMServer");

    // initialize globalization
    Global::initialize();

    // initialize eleaph-proto-RPC-System
    EleaphRpc *eleaphRPC = new EleaphRpc(&a, 65536);

    // initialize 2 packet processor worker threads, which process the packets over it's event loops in parallel!
    for(int i = 0;i < 2; i++) {
        // create new worker thread
        QThread *threadWorker = new QThread(&a);

        // init packet processor and move to worker thread
        PacketProcessor* packetProcessor = new PacketProcessor(eleaphRPC);
        packetProcessor->moveToThread(threadWorker);
        threadWorker->start();
		
		// A part (all submanagers) of the packetprocessor have to be constructed in the worker thread
		// So we let do Qt the job over a Queued method invoke
        QMetaObject::invokeMethod(packetProcessor, "start", Qt::QueuedConnection);
    }

    // start the tcp listening
    eleaphRPC->startTcpListening(Global::getConfigValue("server/port", 0).toInt());

    // start eventloop
    return a.exec();
}
