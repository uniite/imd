import time
#time.sleep(1)

fifo = open("/tmp/oledmgr", "w+")
# Use this to interface to the oled c code by usign a simple pipe to connect the two with teh following statement:
# python -u bridge.py | ./oled
# This isn't ideal nor does this script have a remote access mechanism right now...but its a start

# This is a test routine that should print a single line of small white text saying "Hello World!"
# int8 oled_textbox( int8 x, int8 y, chr text[1000], int8 fontsize, int8 maxwidth, int8 r, int8 g, int8 b, int8 mode, int8 align, int1 inverse)
# stuff = '\x01\x00\x1e\x01\x00\x3f\x3f\x3f\x00\x00\x00'
# text = "Hello World!"
# print stuff
# print text
#while 1 == 1:
#	 pass

def textbox (text, x = 0, y = 0, fontsize = 1, maxwidth = 0, color = [0, 50, 50], mode = 0, align = 0):
	args = chr(x + 40) + chr(y + 40) + chr(fontsize) + chr(maxwidth + 40)
	args += chr(color[0]) + chr(color[1]) + chr(color[2])
	args += chr(mode) + chr(align)
	fifo.write(chr(1) + args + "\n")
	fifo.write(text.replace(" ", chr(175)) + "\n")

def oledinit ():
	fifo.write(chr(5) + "\n")

def redraw (row = 0, row2 = 130, col = 0, col2 = 130):
	args = chr(row) + chr(row2) + chr(col) + chr(col2)
	fifo.write(chr(4) + args + "\n")

def scroll ():
	fifo.write(chr(2) + "\n")
	
def fill (x, y, x2, y2, color):
	args = chr(x + 40) + chr(y + 40) + chr(x2 + 40) + chr(y2 + 40)
	args += chr(color[0]) + chr(color[1]) + chr(color[2])
	fifo.write(chr(3) + args + "\n")

def test ():
	args = chr(7)
	fifo.write(args + "\n")
	
def clear ():
	fifo.write(chr(6) + '\n')

def copy (y, y2, y3, x = 0, x2 = 130, x3 = 0):
	args = chr(x + 40) + chr(y + 40) + chr(x2 + 40) + chr(y2 + 40) + chr(x3 + 40) + chr(y3 + 40)
	fifo.write(chr(8) + args + "\n")
def flush ():
	fifo.flush()
