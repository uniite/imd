#!/usr/bin/python
import os
from time import sleep

os.system("echo lol > fix.txt")

i = 1
print "Try #%s" % i
os.system("./fix.py &")


while 1:
	file = open("fix.txt", "r+")
	result = file.readline()
	file.close()
	sleep(2)
	i += 1
	print "Try #%s: %s" % (i, result)
	if result != "GUI_OK\n":
		os.system("killall fix.py")
		os.system("./fix.py &")
	else:
		os.system("killall fix.py")
		print "done"
		file.close()
		break
