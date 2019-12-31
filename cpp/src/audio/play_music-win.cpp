//
// Created by peter on 12/28/19.
//

#include <string>
#include <Windows.h>
#include <Winnt.h>
#include <stdexcept>
#include <audio/play_music.h>

void play_music(std::string const &filename)
{
	// check the file exists?
	auto const result = PlaySound(filename.c_str(), NULL, SND_FILENAME);
	if (!result) {
		throw std::runtime_error("playing sound failed");
	}
}