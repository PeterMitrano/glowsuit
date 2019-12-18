#!/usr/bin/env python3
"""
LINUX: First run this program then run VMPK. In VMPK, go into connections, and use ALSA and RtMidiIn Client:0
"""
import argparse
import numpy as np
import json
import pathlib
import sys
from time import sleep, time
from typing import Optional

import serial
from PyQt5.QtWidgets import QApplication, QMainWindow
from rtmidi.midiutil import open_midiinput
from xbee import XBee

from command_station_ui import Ui_MainWindow
from visualizer import Visualizer

num_suits = 6


def front_status_clicked(self):
    self.front = not self.front


def back_status_clicked(self):
    self.back = not self.back


class MainUI:

    def __init__(self, ui: Ui_MainWindow, viz: Visualizer):
        self.ui = ui
        self.viz = viz

        self.button_off_style = """
        QPushButton {
            background-color: #e83535;
            color: #000000
        }
        QPushButton:pressed {
            background-color: #ad1010;
            color: #000000
        }
        """
        self.button_on_style = """
        QPushButton {
            background-color: #58eb34;
            color: #000000
        }
        QPushButton:pressed {
            background-color: #34ba13;
            color: #000000
        }
        """

    def setup_ui(self):
        self.ui.front_button.setStyleSheet(self.button_on_style)
        self.ui.back_button.setStyleSheet(self.button_on_style)
        self.ui.front_button.clicked.connect(self.front_status_clicked)
        self.ui.back_button.clicked.connect(self.back_status_clicked)

    def front_status_clicked(self):
        self.viz.front_status_clicked()
        if self.viz.front:
            self.ui.front_button.setStyleSheet(self.button_on_style)
        else:
            self.ui.front_button.setStyleSheet(self.button_off_style)

    def back_status_clicked(self):
        self.viz.back_status_clicked()
        if self.viz.back:
            self.ui.back_button.setStyleSheet(self.button_on_style)
        else:
            self.ui.back_button.setStyleSheet(self.button_off_style)


class MidiHandler:
    def __init__(self, xbee_serial: Optional[serial.Serial],
                 viz: Visualizer,
                 num_channels: int):
        if xbee_serial:
            self.xbee = XBee(xbee_serial)
        else:
            self.xbee = None
        if viz:
            self.viz = viz
            self.no_viz = False
        else:
            self.viz = None
            self.no_viz = True
        self.num_channels = num_channels

        # performance debugging
        self.times = []
        self.dts = []

    def __call__(self, event, data=None):
        message, _ = event
        command = message[0]
        pitch = message[1] - 60
        channel_number = pitch % self.num_channels
        suit_number = pitch // self.num_channels + 1
        xbee_data = [command, channel_number]

        if not self.no_viz:
            self.viz.at(suit_number, command, channel_number)
        if self.xbee:
            t0 = time()
            self.xbee.send('tx', frame_id='\x01', dest_addr=suit_number.to_bytes(2, 'big'), data=bytearray(xbee_data))
            dt = time() - t0
            self.times.append(t0)
            self.dts.append(dt)


def main():
    parser = argparse.ArgumentParser('take MIDI input from a program like VPMK and send it to an XBEE')
    parser.add_argument('suit_file', type=pathlib.Path, help='json file describing suit visualization')
    parser.add_argument('--xbee-port', type=str, default=None)
    parser.add_argument('--keyboard-port', type=int, default=0)
    parser.add_argument('--no-viz', action='store_true')

    args = parser.parse_args()

    suit = json.load(args.suit_file.open('r'))
    num_channels = len(suit['channels'])

    try:
        midiin, port_name = open_midiinput(args.keyboard_port)
    except (EOFError, KeyboardInterrupt):
        sys.exit()

    if args.no_viz:
        viz = None
    else:
        app = QApplication(sys.argv)
        viz = Visualizer(suit, num_channels)
        main_window = QMainWindow()
        ui = Ui_MainWindow()
        ui.setupUi(main_window)
        ui.verticalLayout.addWidget(viz)
        main = MainUI(ui, viz)
        main.setup_ui()
        main_window.show()

    if args.xbee_port:
        ser = serial.Serial(args.xbee_port, 57600)
    else:
        ser = None
    midi_handler = MidiHandler(ser, viz, num_channels)
    midiin.set_callback(midi_handler)

    if args.no_viz:
        while True:
            sleep(10)
            # print(midi_handler.times)
            dts = midi_handler.dts
            print(np.min(dts), np.max(dts), np.median(dts))
    else:
        try:
            return_code = app.exec()
        except KeyboardInterrupt:
            pass

    midiin.close_port()

    sys.exit(return_code)


if __name__ == '__main__':
    main()
