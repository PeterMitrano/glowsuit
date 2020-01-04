#pragma once

#ifdef WIN32
#include <midi/win/midi_input_win.h>
#else
#include <midi/linux/midi_input_linux.h>
#endif