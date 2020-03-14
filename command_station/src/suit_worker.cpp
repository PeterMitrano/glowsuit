#include <QThread>

#include <suit_worker.h>

SuitWorker::SuitWorker(unsigned int suit_idx, QObject *parent)
        : QObject(parent), suit_idx(suit_idx)
{

}

void SuitWorker::start()
{
    while (true)
    {
        // tiny sleep to prevent eating CPU
        QThread::msleep(1);

        if (QThread::currentThread()->isInterruptionRequested())
        {
            break;
        }

        // send a message to the visualizer
//        emit midi_event(suit_number, command, channel_number);
    }
}

