# take in a MIDI file, and for any messages that are supposed to be simultaneous, pack them up into one bitstring
# and transmit them via serial to the master Xbee.

import argparse
from mido import MidiFile
import serial

from time import sleep, time
from xbee import XBee


def main():
    parser = argparse.ArgumentParser("play back midi file and send to the real suits")
    parser.add_argument('midi_file')

    args = parser.parse_args()

    file = MidiFile(args.midi_file)

    if len(file.tracks) > 1:
        print("which track do you want?")
        track_names = [track.name for track in file.tracks]
        for i, track_name in enumerate(track_names):
            print("{}) {}".format(i, track_name))
        while True:
            response = input("enter number:")
            try:
                j = int(response)
                break
            except ValueError:
                print("not a valid number, try again")
                pass
        choreo_track = file.tracks[j]
    else:
        choreo_track = file.tracks[0]

    for msg in choreo_track:
        if msg.is_meta:
            continue


if __name__ == '__main__':
    main()
