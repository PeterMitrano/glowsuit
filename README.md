# Overview

To write choreo, you'll need LMMS or Ableton (or any other DAW should work). The DAW will be responsible for playing the music and the MIDI which controls the suits. The DAW gives you a nice way to write the MIDI and line everything up with the music. The `command_station` is responsible for visualizing the choreo, and for sending it via XBee to the suits.

Come performance time, we will (hopefully?) be using the `command_station` to both play the music and the choreo. This is done by giving it a MIDI file (the choreo) and a WAV file (the music). Then the `command_station` program will start them at the same time. It also does some fancier things to improve performance/latency.

**NOTE:** you can hover over things in the GUI to see little hints.

# Installation

## Windows

1. Install Loop-MIDI: [http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip](http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip)
1. Download the `command_station.exe` and the `suit.json` file from the [latest release here on github](https://github.com/PeterMitrano/glowsuit/releases/latest).
1. Put the executable and the json file in the same folder, and double click to run the exe file.


## Ubuntu Linux

Install Qt and Qt multimedia plugins.

    sudo apt install qt5-default libqt5multimedia5-plugins

You should now be able to run the linux binary. If you want to compile it yourself, you may need extra libraries.
