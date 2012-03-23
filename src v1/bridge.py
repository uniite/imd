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
#    pass
    

    
def textbox (x, y, text, fontsize, maxwidth, color, mode, align, inverse):
    args = chr(x) + chr(y) + chr(fontsize) + chr(maxwidth)
    args += chr(color[0]) + chr(color[1]) + chr(color[2])
    args += chr(mode) + chr(align) + chr(inverse)
    fifo.write(chr(1) + args + "\n")
    fifo.write(text + "\n")
    
def oledinitz ():
    fifo.writelines(chr(5) + "\n")

def flush ():
    fifo.writelines(chr(4) + "\n")

def scroll ():
    fifo.writelines(chr(2) + "\n")

def fill (x, y, x2, y2, color):
    args = chr(x) + chr(y) + chr(x2) + chr(y2)
    args += chr(color[0]) + chr(color[1]) + chr(color[2])
    fifo.writelines(chr(3) + args + "\n")
    

def test ():
	args = chr(7)
	fifo.writelines(args + "\n")
#textbox(0, 30, "Hello_World!", 1, 0, [0, 63, 0], 0, 0, 0)
def clear():
   fifo.write(chr(6) + '\n')
