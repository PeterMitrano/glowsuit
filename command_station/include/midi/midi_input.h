#pragma once

#ifdef WIN32
#include <midi/win/midi_input_win.h>
#elif __APPLE__
#include <midi/osx/midi_input_osx.h>
#elif __linux__
#include <midi/linux/midi_input_linux.h>
#endif
