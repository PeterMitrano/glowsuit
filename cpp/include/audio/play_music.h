#pragma once

#include <QObject>
#include <QString>

class MusicWorker : public QObject
{
	Q_OBJECT

public:
	MusicWorker(QObject* parent = nullptr);

public slots:
	void play_music(QString music_filename);

signals:
	void my_finished();

private:
	QString music_filename;

};

