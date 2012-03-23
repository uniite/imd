from time import time, sleep
file = open("/root/uptime", "r+")
start = int("".join(file.readlines()))
file.close()
while 1:
	file = open("/root/uptime", "w+")
	file.write(str(int(time() + start)))
	file.close()
	