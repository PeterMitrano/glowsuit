#!/usr/bin/env python
import serial


ser = serial.Serial("/dev/pts/3")
while True:
    print(ser.read(6))
