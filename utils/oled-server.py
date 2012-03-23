#!/usr/bin/python
from socket import *
import os, re, sys, threading

response = """HTTP/1.1 200 OK\nServer: JB Python Server (Gumstix)\nConnection: close\n\n"""
header = "BM\x46\xC7\0\0\0\0\0\0\x36\0\0\0\x28\0\0\0\x82\0\0\0\x82\0\0\0\x01\0\x18\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#file = open("I:\\Jon-xp backup\\C\\Documents and Settings\\JB\\My Documents\\desktop\\gumstix\\test.bmp", "r+")
#data = eval('"%s"' % "".join(file.readlines()))
#file.close()

re_url = re.compile("GET /(.*?) HTTP/1.1")
re_port = re.compile("http://gumstix.local:808[0-9]")

oled = open("/tmp/oledsvr", "r+")

class oledmgr_thread (threading.Thread):
	def run (self):
		global data
		i = 0
		while 1:
			#print "Waiting..."
			temp = oled.readline()
			if temp[:10] == "!oled-sync":
				print "Trying to sync"
				data = oled.readline()[:-1]
				print len(data)
				print "Working..."
				data = eval('"' + data + '"')
				#data =  eval('"%s"' % oled.read(203840))
				print len(data)
				print "Frame Sync"
					#if temp == "oled-sync":
						#break
					#elif temp.find("\\x") == -1:
						#sys.stdout.write("!" + temp)
			elif temp.find("\\x") == -1:
				sys.stdout.write(temp)
			#temp = oled.read(203840)
			#if len(temp) > 0:
				#data =  eval('"%s"' % temp)
				#print len(data)
			#print "oledmgr_thread cycle #%s" % i
			i += 1
				
class oledsvr_thread (threading.Thread):
	def run (self):
		global data, port
		#print "server thread start"
		server = socket(AF_INET, SOCK_STREAM)
		try:
			port = 8080
			server.bind(("", port))
		except:
			try:
				port = 8081
				server.bind(("", port))
			except:
				port = 8082
				server.bind(("", port))
		#html = open("/var/www/index.htm", "r+")
		#temp = "".join(html.readlines())
		#html.close()
		#temp = re_port.sub("http://gumstix.local:808[0-9]", temp)
		#html = open("/var/www/index.htm", "w+")
		#html.write(temp)
		#html.close()
		print "Server on port %s" % port
		server.listen(5)
		i = 0
		while 1:
			con, addr = server.accept()
			req = con.recv(4096)
			#open("req.txt", "w+").write(req)
			con.send(response + header + data)
			con.close()
			print "HTTP request #%s" % i
			i += 1

oledmgr_thread().start()
oledsvr_thread().start()
os.system("sed -i \"s/gumstix.local:808./gumstix.local:%s/g\" /var/www/index.htm" % port)