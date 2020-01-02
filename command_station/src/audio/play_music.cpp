//
// Created by peter on 12/28/19.
//

#include <string>

#include <QMediaPlayer>

#include <audio/play_music.h>

MusicWorker::MusicWorker(QObject *parent)
	: QObject(parent)
{}

void MusicWorker::play_music(QString music_filename)
{
	QMediaPlayer player;
	//QObject::connect(&player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChanged(qint64)));
	player.setMedia(QUrl::fromLocalFile(music_filename));
	player.setVolume(100);
	player.play();
}