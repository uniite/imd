def check_input ():
	global X
	global Y
	global Xavg
	global Yavg
	global Xavgc
	global Yavgc
	c = 0
	avg = 0
	for i in range(0, 15):
		#tty.write("x")
		X = eval(tty.read(30)[4:9])
		if X < 700 and X > 300:
			avg += X
			c += 1
	print "XAvg: %s" % (avg / c)
	X = avg / c
	print "Xavg: %s" % (Xavg / Xavgc)
	if X  > (Xavg / Xavgc) + 15:
		menu_back()
		sleep(.4)
		Yavg += Y
		Yavgc += 1
		return
	elif X < (Xavg / Xavgc) - 15:
		menu_action()
		sleep(.4)
		Yavg += Y
		Yavgc += 1
		return
	else:
		Xavg += X * 2
		Xavgc += 2
	c = 0
	avg = 0
	for i in range(0, 5):
		#tty.write("y")
		Y = eval(tty.read(30)[14:19])
		if Y < 700 and Y > 300:
			avg += Y
			c += 1
	Y = avg / c
	print "YAvg: %s" % (avg / c)
	print "Yavg: %s" % (Yavg / Yavgc)
	if Y < (Yavg / Yavgc) - 15:
		menu_down()
	elif Y > (Yavg / Yavgc) + 15:
		menu_up()
	else:
		Yavg += Y * 2
		Yavgc += 2

# Claibrate Accelerometer
def calibrate ():
	global X
	global Y
	global Xavg
	global Yavg
	global Xavgc
	global Yavgc
	Xavg = 0
	Yavg = 0
	Xavgc = 0
	Yavgc = 0
	i = 5
	while i > -1:
		oled.clear()
		oled.flush()
		oled.textbox(x = 40, y = 50, text = "Calibrating in %s" % i)
		oled.flush()
		oled.redraw()
		oled.flush()
		sleep(1)
		i -= 1
	i = 250
	i2 = 0
	while i > 0:
		tty.write("x")
		s = tty.read(9)
		X = eval(s[4:])
		if X < 700 and X > 300:
			Xavg += X
			Xavgc += 1
			print "Xavg: " + str(Xavg / Xavgc)
		tty.write("y")
		s = tty.read(9)
		Y = eval(s[4:])
		if Y < 700 and Y > 300:
			Yavg += Y
			Yavgc += 1
			print "Yavg: " + str(Yavg / Yavgc)
		if i2 == 30:
			oled.clear()
			oled.flush()
			oled.textbox(x = 35, y = 50, text = "%s percent done..." % ((250 - i) / 2.5))
			oled.flush()
			oled.redraw()
			oled.flush()
			i2 = 0
		sleep(0.02)
		i2 += 1
		i -= 1
	menu_home()