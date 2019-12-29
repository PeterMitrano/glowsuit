#!/usr/bin/env python
import serial
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("port")
args = parser.parse_args()
ser = serial.Serial(args.port)
while True:
    print(ser.read(6))
