import os, time

os.system("aumix -v 50")
volume = 50

mp3opened = 0
mp3playing = 0
mpfifos = []

# Take complete control of a console app
def ctlapp (app):
	global mp3opened, mp3playing, mpfifos
	mp3opened = 0
	mp3playing = 0
	mpfifos = []
	fifos = ["stdin", "stdout", "stderr"]
	i = 0
	fptr = [""] * 3
	for i in range(0, 3):
		#fifos[i] = "/tmp/" + app + "-" + fifos[i]
		fifos[i] = "/tmp/%s-%s" % (app.split()[0], fifos[i])
		os.system("rm " + fifos[i])
		os.system("mkfifo " + fifos[i])
		print fifos[i]
	mpfifos.append(open(fifos[0], "w+"))
	mpfifos.append(open(fifos[1], "r+"))
	mpfifos.append(open(fifos[2], "r+"))
	#os.system (app + " < " + fifos[0] + " > " + fifos[1] + " 2> " + fifos[2]
	#pid = os.system (os.P_NOWAIT, "%s < %s > %s 2> %s &" % (app, fifos[0], fifos[1], fifos[2]))
	mpfifos[1].close()
	os.system("%s < %s > %s 2> %s &" % (app, fifos[0], fifos[1], fifos[2]))
	os.system("cat %s > /dev/null &" % fifos[1])

def mp3init ():
	ctlapp ("mpg321 -o oss -R blah")

def mp3open (file):
	global mp3opened
	if mp3opened == 1:
		mp3close()
	mp3init()
	mpfifos[0].write("LOAD %s\n" % file)
	mpfifos[0].flush()
	mpfifos[0].flush()
	mp3opened = 1
	mp3playing = 1

# Note: all mpg321 commands can be shortened to 
def mp3pause ():
	global mp3opened, mp3playing
	if mp3opened == 1:
		if mp3playing:
			vol_mute()
			mp3playing = 0
		else:
			vol_unmute()
			mp3seek(-1)
			mpg3playing = 1
		mpfifos[0].write("PAUSE\n")
		mpfifos[0].flush()
		mpfifos[0].flush()

def mp3seek (val):
	global mp3opened, mp3playing
	if mp3opened == 1:
		mpfifos[0].write("JUMP %s\n" % val)
		mpfifos[0].flush()
		if mp3playing:
			mp3playing = 0
		else:
			mpg3playing = 1

def mp3close ():
	global mp3opened
	if mp3opened == 1:
		mpfifos[0].write("QUIT\n")
		mpfifos[0].flush()
		mpfifos[0].flush()
		mp3playing = mp3opened = 0
		os.system("killall mpg321 cat")

def mp3getpos ():
	global mp3opened
	if mp3opened == 1:
		mpfifos[1] = open(mpfifos[1].name, "r+")
		temp = mpfifos[1].readline().split()
		# temp = ["@F", frames elapsed, frames left, seconds elapsed, seconds left]
		return temp

def vol_up ():
	global volume
	volume += 1
	os.system("aumix -v " + volume)

def vol_down ():
	global volume
	volume -= 1
	os.system("aumix -v " + volume)

def vol_mute():
	os.system("aumix -v 0")

def vol_unmute():
	os.system("aumix -v " + volume)