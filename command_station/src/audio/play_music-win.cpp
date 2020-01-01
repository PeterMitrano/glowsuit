//
// Created by peter on 12/28/19.
//

#include <string>
#include <Windows.h>
#include <Winnt.h>
#include <stdexcept>
#include <audio/play_music.h>

MusicWorker::MusicWorker(QObject *parent)
	: QObject(parent)
{}

void MusicWorker::play_music(QString music_filename)
{
	// TODO: this might not be nessecary
	this->music_filename = music_filename;

	// check the file exists?
	// TODO: replace this with non-blocking multimedia control
	auto const result = PlaySound(music_filename.toStdString().c_str(), NULL, SND_FILENAME);
	if (!result) {
		throw std::runtime_error("playing sound failed");
	}
}