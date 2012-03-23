#!/usr/bin/python
import os
from bintoint import bintoint
output = "int8 font_arial[224][13] = {\n"
for i in range(32, 256):
	os.system("ttfbanner -p 9 arial.ttf 'j %s j' > ttf2imd_temp.txt" % chr(i))
	ttf = open("ttf2imd_temp.txt", "r+")
	data = ttf.readlines()
	ttf.close()
	glyph = []
	width = 0
	for line in data:
		temp = line[2:-2].replace("*", "1").replace(" ", "0")
		glyph.append(temp)
		if len(line[2:-2].strip()) > width:
			width = len(line[2:-2].strip())
			key = temp
	triml = key.find("1")
	trimr = key.rfind("1") + 1
	output += "\t{ // ASCII %sd(%s) = %s\twidth: %s\n" % (i, hex(i), chr(i), width)
	for num in glyph:
		output += "\t\t%s,\n" % hex(bintoint(num[triml:trimr]))
	output += "\t},\n"
output += "};\n"
font = open("font_arial.h", "w+")
font.write(output)
font.close()