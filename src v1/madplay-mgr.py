from os import system
file = open("/root/uptime", "r+")
system("madplay /mnt/mmc/music/* -s %s &" % int("".join(file.readlines())))
system("python /root/uptime.py &")
file.close()