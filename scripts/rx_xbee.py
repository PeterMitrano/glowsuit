#!/usr/bin/env python3
import argparse
from time import sleep

import serial
from xbee import XBee


def main():
    parser = argparse.ArgumentParser("test XBEE")
    parser.add_argument('--xbee-port', type=str, default='/dev/ttyUSB4')

    args = parser.parse_args()

    ser = serial.Serial(args.xbee_port, 57600)
    xbee = XBee(ser)
    while True:
        try:
            response = xbee.wait_read_frame()
            print(str(response['rf_data']))
        except Exception:
            pass


if __name__ == '__main__':
    main()
