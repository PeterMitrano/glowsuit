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
from PyQt5.QtWidgets import QApplication, QMainWindow, QLabel, QWidget
from rtmidi.midiutil import open_midiinput
from xbee import XBee

num_suits = 6


class Visualizer(QWidget):
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
        self.suit = suit
        self.setMinimumSize(self.width, self.height)
        self.off_color = QColor(50, 50, 50, 255)
        self.num_channels = num_channels
        self.on_channels = np.zeros([num_suits, self.num_channels], dtype=np.bool)
        self.front = True
        self.back = True

    def front_status_clicked(self):
        self.front = not self.front

    def back_status_clicked(self):
        self.back = not self.back

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
