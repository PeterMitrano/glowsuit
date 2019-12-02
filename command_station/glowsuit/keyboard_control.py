#!/usr/bin/env python3
"""
LINUX: First run this program then run VMPK. In VMPK, go into connections, and use ALSA and RtMidiIn Client:0
"""
import argparse
import numpy as np
import json
import pathlib
import sys
from dataclasses import dataclass
from typing import List

from PyQt5 import QtCore
from PyQt5.QtGui import QPainter, QColor
from PyQt5.QtWidgets import QApplication, QMainWindow
from rtmidi.midiutil import open_midiinput
from xbee import XBee

num_suits = 6


@dataclass()
class LineData:
    x0: int
    y0: int
    x1: int
    y1: int


@dataclass()
class Circle:
    x0: int
    y0: int
    r: int


class Visualizer(QMainWindow):
    def __init__(self, suit):
        super().__init__()
        self.title = "Glowsuit Visualizer"
        self.top = 50
        self.left = 50
        self.width = 500
        self.height = 500
        self.setWindowTitle(self.title)
        self.setGeometry(self.top, self.left, self.width, self.height)
        # TODO: set background to black
        self.show()
        self.suit = suit
        self.off_color = QColor(0, 0, 0, 50)

        self.num_channels = len(suit['channels'])
        self.on_channels = np.zeros([num_suits, self.num_channels], dtype=np.bool)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        for suit_idx in range(num_suits):
            sx = 50 + 100 * suit_idx
            sy = 25
            for channel_idx, channel_description in enumerate(self.suit['channels']):
                if 'lines' in channel_description:
                    for line in channel_description['lines']:
                        if self.on_channels[suit_idx, channel_idx]:
                            r, g, b = line['color']
                            color = QColor(r, g, b, 255)
                        else:
                            color = self.off_color
                        painter.setPen(color)
                        painter.drawLine(sx + line['x1'], sy + line['y1'], sx + line['x2'], sy + line['y2'])
                if 'circles' in channel_description:
                    for circle in channel_description['circles']:
                        if self.on_channels[suit_idx, channel_idx]:
                            r, g, b = circle['color']
                            color = QColor(r, g, b, 255)
                        else:
                            color = self.off_color
                        painter.setPen(color)
                        painter.drawEllipse(sx + circle['x'], sy + circle['y'], circle['r'], circle['r'])

    def at(self, message: List[int]):
        command = message[0]

        # FIXME: magic constant
        pitch = message[1] - 60

        channel_number = pitch % self.num_channels
        suit_number = pitch // self.num_channels

        if 0 <= suit_number <= num_suits and 0 <= channel_number <= 7:
            if command == 128:  # off
                print(channel_number, suit_number, 'off')
                self.on_channels[suit_number, channel_number] = 0
            elif command == 144:  # on
                print(channel_number, suit_number, 'on')
                self.on_channels[suit_number, channel_number] = 1
            else:
                return # just ignore any other types of midi messages

            self.update()


class MidiToXBee:
    def __init__(self, xbee_port, viz):
        self.xbee = XBee(xbee_port)
        self.viz = viz

    def __call__(self, event, data=None):
        message, _ = event
        self.viz.at(message)
        # self.xbee.at(command=message)


def main():
    parser = argparse.ArgumentParser('take MIDI input from a program like VPMK and send it to an XBEE')
    parser.add_argument('suit_file', type=pathlib.Path, help='json file describing suit visualization')
    parser.add_argument('--xbee-port', type=str, default='/dev/ttyUSB0')
    parser.add_argument('--keyboard-port', type=int, default=0)

    args = parser.parse_args()

    suit = json.load(args.suit_file.open('r'))

    try:
        midiin, port_name = open_midiinput(args.keyboard_port)
    except (EOFError, KeyboardInterrupt):
        sys.exit()

    app = QApplication(sys.argv)

    viz = Visualizer(suit)
    midi_to_xbee = MidiToXBee(args.xbee_port, viz)
    midiin.set_callback(midi_to_xbee)

    return_code = app.exec()

    midiin.close_port()

    sys.exit(return_code)


if __name__ == '__main__':
    main()
