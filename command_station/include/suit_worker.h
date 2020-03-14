#pragma once

#include <QObject>

class SuitWorker : public QObject
{
Q_OBJECT

public:
    SuitWorker(unsigned int suit_idx, QObject *parent = nullptr);

public slots:

    void start();

signals:

    void midi_event(unsigned int suit_number, unsigned int command, unsigned int channel_number);

    void my_finished();

private:
    unsigned int suit_idx;

};

