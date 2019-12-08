# Installation

## Windows

### Setup

Install MIDI-OX: [http://www.midiox.com/moxdown.htm](http://www.midiox.com/moxdown.htm)
Install Loop-MIDI: [http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip](http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip)
Install our python environment [https://drive.google.com/open?id=1yHCn4LRKgTvRy00oHYm2odFBysq8-_aw](https://drive.google.com/open?id=1yHCn4LRKgTvRy00oHYm2odFBysq8-_aw).
1. Extract the zip
1. Open command prompt
1. go to the folder, using the `cd` command. For instance, `cd C:/Users/peter/Documents/glowsuit-windows
1. still in command prompt, run `Scripts\activate.bat`. If all goes well, you should see (glowsuit) printed at the begging of tne in the command prompt.
1. Run `conda-unpack`
1. Download this repository and put it in that same folder

### Running The Program

This part is complicated, so follow these steps carefully.
1. Run loopMIDI. If it's already running, great. Hit the plus button and you should see something like this (TODO image)
1. From the command prompt, in the glowsuit folder, run `python command_station/glowsuit/command_station.py --keyboard-port 100`. At this point it should promp you to enter a number based on which midi port you want. Enter the number corresponding to loopmidi. The gui should then open. You can also run the program like `python command_station/glowsuit/command_station.py --keyboard-port 100 --xbee-port COM3` where `COM3` is whichever port you plugged the XBee into.
1. To pay back the choreography, run MIDI Bar (windows search should find it, it's part of MIDI OX). Click the midi devices button, and select loop midi. The click the folder button and open the choreo MIDI file, then hit play.
