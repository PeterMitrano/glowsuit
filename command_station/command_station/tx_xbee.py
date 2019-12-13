#!/usr/bin/env python3
import argparse
import time
from time import sleep

import serial
from xbee import XBee


def main():
    parser = argparse.ArgumentParser("test XBEE")
    parser.add_argument('--xbee-port', type=str, default='/dev/ttyUSB1')

    args = parser.parse_args()
    ser = serial.Serial(args.xbee_port, 57600)
    xbee = XBee(ser)

    while True:
        time.sleep(2)
        xbee.send('tx', frame_id='\x01', dest_addr='\x00\x01', data=b'\x80\x3C\xFF')
        response = xbee.wait_read_frame(timeout=5)
        print(response)


if __name__ == '__main__':
    main()
