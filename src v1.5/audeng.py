import os, time
import binascii
from time import sleep

volume = 50
os.system("aumix -v %s" % volume)

mode = "stopped"

fifo = open("audengfifo", "wb+")
fifo2 = open("audengfifo2", "rb+")

# Convert int to little endian binary (16bit)
def binLE (num):
	lile = chr(num & 0xFF)
	lile += chr(num >> 8)
	return lile

def play (filename):
	global mode
	if mode != "stopped":
		stop()
	fifo.write("o%s%s\0" % (binLE((len(filename) + 1)), filename))
	fifo.flush()
	mode = "playing"

def pause ():
	global mode
	if mode == "paused":
		resume()
	elif mode == "playing":
		fifo.write("p")
		fifo.flush()
		mode = "paused"

def resume ():
	global mode
	if mode == "paused":
		fifo.write("r")
		fifo.flush()
		mode = "playing"

def seek (position):
	global mode
	if mode != "stopped":
		fifo.write("s%s" % binLE(position))
		fifo.flush()

def stop ():
	global mode
	if mode != "stopped":
		fifo.write("c")
		fifo.flush()
		mode = "stopped"
		sleep(1)

def getpos ():
	global mode
	if mode != "stopped":
		fifo.write("g")
		fifo.flush()
		return [ord(fifo2.read(1)) + (ord(fifo2.read(1)) << 8), ord(fifo2.read(1)) + (ord(fifo2.read(1)) << 8)]
	else:
		return 0

def vol_up ():
	global volume
	volume += 1
	os.system("aumix -v %s" % volume)

def vol_down ():
	global volume
	volume -= 1
	os.system("aumix -v %s" % volume)

def vol_mute():
	os.system("aumix -v 0")

def vol_unmute():
	os.system("aumix -v %s" % volume)