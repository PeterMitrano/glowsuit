#!/usr/bin/env python3
"""
LINUX: First run this program then run VMPK. In VMPK, go into connections, and use ALSA and RtMidiIn Client:0
"""
import argparse
import json
import pathlib
import sys
from typing import Optional

import numpy as np
import serial
from PyQt5 import QtCore
from PyQt5.QtCore import QEvent
from PyQt5.QtGui import QPainter, QColor
from PyQt5.QtWidgets import QApplication, QMainWindow
from rtmidi.midiutil import open_midiinput
from xbee import XBee

num_suits = 6

appStyle = """
QMainWindow{
background-color: darkgray;
}
"""


class Visualizer(QMainWindow):

    def __init__(self, suit, num_channels: int):
        super().__init__()
        self.title = "Glowsuit Visualizer"
        self.offset_x = 10
        self.offset_y = 10
        self.suit_width = 100
        self.width = num_suits * self.suit_width + self.offset_x * 2
        self.height = 200
        self.left = 1366 - self.width - 10
        self.top = 10
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        self.setStyleSheet(appStyle)
        self.suit = suit
        self.off_color = QColor(0, 0, 0, 50)
        self.num_channels = num_channels
        self.on_channels = np.zeros([num_suits, self.num_channels], dtype=np.bool)
        self.front = True
        self.back = True

        self.show()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)
        for suit_idx in range(num_suits):
            sx = self.offset_x + self.suit_width * suit_idx
            sy = self.offset_y
            for channel_idx, channel_description in enumerate(self.suit['channels']):
                if 'lines' in channel_description:
                    for line in channel_description['lines']:
                        color = self.off_color
                        r, g, b = line['color']
                        if self.on_channels[suit_idx, channel_idx]:
                            if (self.back and line['back']) or (self.front and line['front']):
                                color = QColor(r, g, b, 255)
                        painter.setPen(color)
                        painter.drawLine(sx + line['x1'], sy + line['y1'], sx + line['x2'], sy + line['y2'])

                if 'circles' in channel_description:
                    for circle in channel_description['circles']:
                        color = self.off_color
                        r, g, b = circle['color']
                        if self.on_channels[suit_idx, channel_idx]:
                            if (self.back and circle['back']) or (self.front and circle['front']):
                                color = QColor(r, g, b, 255)
                        painter.setPen(color)
                        painter.drawEllipse(sx + circle['x'], sy + circle['y'], circle['r'], circle['r'])

    def keyPressEvent(self, event):
        if event.key() == QtCore.Qt.Key_Q:
            self.deleteLater()
        if event.key() == QtCore.Qt.Key_F and event.type() == QEvent.KeyPress:
            # enable drawing of the front of the suits
            self.front = not self.front
            event.accept()
        if event.key() == QtCore.Qt.Key_B and event.type() == QEvent.KeyPress:
            # enable drawing of the back of the suits
            self.back = not self.back
            print(self.back)
            event.accept()

    def at(self, suit_number: int, command: int, channel_number: int):
        if 1 <= suit_number <= num_suits and 0 <= channel_number <= 7:
            if command == 128:  # off
                # print(channel_number, suit_number, 'off')
                self.on_channels[suit_number - 1, channel_number] = 0
            elif command == 144:  # on
                # print(channel_number, suit_number, 'on')
                self.on_channels[suit_number - 1, channel_number] = 1
            else:
                return  # just ignore any other types of midi messages

            self.update()


class MidiHandler:
    def __init__(self, xbee_serial: Optional[serial.Serial],
                 viz: Visualizer,
                 num_channels: int):
        if xbee_serial:
            self.xbee = XBee(xbee_serial)
        else:
            self.xbee = None
        self.viz = viz
        self.num_channels = num_channels

    def __call__(self, event, data=None):
        message, _ = event
        command = message[0]
        pitch = message[1] - 60
        channel_number = pitch % self.num_channels
        suit_number = pitch // self.num_channels + 1
        xbee_data = [command, channel_number]

        self.viz.at(suit_number, command, channel_number)
        if self.xbee:
            self.xbee.send('tx', frame_id='\x01', dest_addr=suit_number.to_bytes(2, 'big'), data=bytearray(xbee_data))


def main():
    parser = argparse.ArgumentParser('take MIDI input from a program like VPMK and send it to an XBEE')
    parser.add_argument('suit_file', type=pathlib.Path, help='json file describing suit visualization')
    parser.add_argument('--xbee-port', type=str, default=None)
    parser.add_argument('--keyboard-port', type=int, default=0)

    args = parser.parse_args()

    suit = json.load(args.suit_file.open('r'))
    num_channels = len(suit['channels'])

    try:
        midiin, port_name = open_midiinput(args.keyboard_port)
    except (EOFError, KeyboardInterrupt):
        sys.exit()

    app = QApplication(sys.argv)

    viz = Visualizer(suit, num_channels)
    if args.xbee_port:
        ser = serial.Serial(args.xbee_port, 57600)
    else:
        ser = None
    midi_handler = MidiHandler(ser, viz, num_channels)
    midiin.set_callback(midi_handler)

    try:
        return_code = app.exec()
    except KeyboardInterrupt:
        pass

    midiin.close_port()

    sys.exit(return_code)


if __name__ == '__main__':
    main()
