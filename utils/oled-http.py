#!/usr/bin/python
from socket import *

header = "HTTP/1.1 200 OK\nContent-type: text/html\n\n"

server = socket(AF_INET, SOCK_STREAM)
server.bind(("", 8085))
server.listen(5)

while 1:
	con, addr = server.accept()
	file = open("/var/www/index.htm", "r+")
	con.send(header + "".join(file.readlines()))
	con.close()
	