# Overview

To write choreo, you'll need LMMS or Ableton (or any other DAW should work). The DAW will be responsible for playing the music and the MIDI which controls the suits. The DAW gives you a nice way to write the MIDI and line everything up with the music. The `command_station` is responsible for visualizing the choreo, and for sending it via XBee to the suits.

Come performance time, we will (hopefully?) be using the `command_station` to both play the music and the choreo. This is done by giving it a MIDI file (the choreo) and a WAV file (the music). Then the `command_station` program will start them at the same time. It also does some fancier things to improve performance/latency.

**NOTE:** you can hover over things in the GUI to see little hints.

# Dependencies

DON'T SKIP THIS.

## Mac
1. Install the FTDI driver so you can talk to the XBees https://www.ftdichip.com/Drivers/VCP.htm

## Windows

1. Install Loop-MIDI: [http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip](http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip)

## Ubuntu (Bionic)

Install Qt and Qt multimedia plugins via the [online installer](https://www.qt.io/download-qt-installer). You may have to check "Latest Release" and hit refresh to see version 5.14. You'll also need least libstdc++6 version 3.4.22.

You should now be able to run the linux binary. If you want to compile it yourself, you may need extra libraries.

## Getting the Running the Program


1. Download the `command_station.exe` and the `suit.json` file from the [latest release here on github](https://github.com/PeterMitrano/glowsuit/releases/latest).
1. Put the executable and the json file in the same folder, and run the executable.

# ISSUES/TODO

- reduce latency
- measure round-trip XBee time, Xbee throughput, etc..
