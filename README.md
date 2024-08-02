:warning: These instructions are for a previous version of the glowsuits, from ~2020. :warning:

# Overview

To write choreo, you'll need LMMS or Ableton (or any other DAW should work). The DAW will be responsible for playing the music and the MIDI which controls the suits. The DAW gives you a nice way to write the MIDI and line everything up with the music. The `command_station` is responsible for visualizing the choreo, and for sending it via XBee to the suits.

Come performance time, we will (hopefully?) be using the `command_station` to both play the music and the choreo. This is done by giving it a MIDI file (the choreo) and a WAV file (the music). Then the `command_station` program will start them at the same time. It also does some fancier things to improve performance/latency.

**NOTE:** you can hover over things in the GUI to see little hints.

# Dependencies

DON'T SKIP THIS. If you do, the GUI will not even open.

## Windows

1. Install Loop-MIDI: [http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip](http://www.tobias-erichsen.de/wp-content/uploads/2018/12/loopMIDISetup_1_0_15_26.zip)
2. 
## Mac
1. Install the FTDI driver so you can talk to the XBees https://www.ftdichip.com/Drivers/VCP.htm

## Ubuntu (18.04, 20.04, ...)

Install Qt and Qt multimedia plugins via the [online installer](https://www.qt.io/download-qt-installer). You may have to check "Latest Release" and hit refresh to see version 5.14. You'll also need least libstdc++6 version 3.4.22.

You should now be able to run the linux binary. If you want to compile it yourself, you may need extra libraries.

## Running the Program

1. Download the `command_station.exe` and the `suit.json` file from the [latest release here on github](https://github.com/PeterMitrano/glowsuit/releases/latest).
1. Put the executable and the json file in the same folder, and run the executable.


# How it works

Devices:
- Suits. each have a microcontroller on them
- Laptop. this runs the `command_station` and controls everything. First you write choreography in a DAW, and save it was a midi file. Then you convert the midi file into code for the suits. The code is also used by the command station but that's less important. Then, you upload that code to each of the suits. Finally, you use the laptop to command the suits to start.

# Generating choreo and uploading

Once you have the .mid file that contains the choreo, you need to convert it to the data that gets uploaded to the suits. To do that run

    ./scripts/midi_to_code.py path/to/choreo.mid arduino/suits command_station

That will generate a bunch of files like `arduino/suits/suit_2.cpp`. Then you need to upload them to the suits. FIRST, turn the little switch on the side of the red circuit board to "FTDI", not "XBEE".

    make SERIAL_PORT=/dev/ttyUSB0 upload-suit2

You may need to change the USB port. To make it easy, make sure the XBee isn't plugged in. Once you hit upload, press the reset button on the suit. The lights on the suit should turn off and stay until the upload finishes, which should take ~10 seconds. If the lights turn right back on, you need to reset the suit again or try re-uploading. The timing is a little annoying and it might take a few tries. Finally, remember to put the switch back to XBEE, otherwise the suit will not start its choreo when you ask it to.
# ISSUES/TODO

- reduce latency
- measure round-trip XBee time, Xbee throughput, etc..
