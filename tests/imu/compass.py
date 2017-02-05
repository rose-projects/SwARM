#!/usr/local/bin/python

from visual import *

previous_angle = 0
angle = 0
Pi = 3.14159265359

pointer = arrow(pos=(0,-5,0), axis=(0,15,0), shaftwidth=1)

SerialPort = '/dev/cu.usbmodem311'

while 1 :
	rate(10)

	with open(SerialPort) as f:
		line = f.readline()
		num = line.split(" : ")
		
		try:
			previous_angle = angle
			angle = float(num[1])
		except:
			continue

	pointer.rotate(angle= angle - previous_angle, axis=(0, 0, 1), origin=(0,0,0))

f.close()
