# Import custom OLED and Menu Modules (which will load the music database module)
print "importing oled"
import oled
print "importing menu"
import menu
print "importing threading"
import threading
print "importing re"
import re
print "importing audeng"
import audeng
print "importing os"
import  os
print "importing time"
from time import sleep

print "initalizing vars"

# Constants..pretty much
row_height = 13
menu_startrow = 1
menu_rows = 8
offset = 3
# equals 16, leaves two rows above
menu_start = menu_startrow * row_height + offset
# equals 120 (bottom of last entry), leaves an extra row below
menu_end = menu_rows * row_height + menu_startrow * row_height

# Default mode for browsing music..other mode is play (noew playing screen),
mode = "browse"

green = [0, 53, 0]
orange = [63, 53, 16]
pblue = [30, 45, 63]
black = [0, 0, 0]
blue = [0, 30, 50]

class highlight_color:
	pass
class border_color:
	pass
	

# Current menu, duh :-P...see menu_load for more info
class curmenu:
	pass

# List of songs in current playlist (holds ids only, no filename)
# *For now it will just hold (possibly shuffled) list of all songs
playlist = []
results = menu.imd_db.select()
for item in results:
	playlist.append(item)
# Index of current song in playlist
cursong = 0

try:
	cursongfile = open("cursong", "r+")
	x = file.readlines()
	cursong = int(x)
	cursongfile.close()
except:
	cursong = 0

hold = 0
input_mode = "accel"

def menu_load (data, index = 0, cursor = menu_start, top = 0):
	# Until this is better coded, if you specify an index, you MUST specify a cursor position, AND a top
	curmenu.name = data[0]
	curmenu.items = data[1]
	curmenu.len = len(curmenu.items)
	curmenu.index = index
	curmenu.cursor = cursor
	curmenu.top = top
	if curmenu.len < menu_rows:
		# If the menu isn't big enough to fill the screen,
		# find out where it ends on screen (y position)
		curmenu.end = curmenu.len * row_height + menu_start
	else:
		curmenu.end = menu_end
	menu_draw()
	oled.redraw()
	oled.flush()

def menu_draw ():
	# Clear the frame buffer first
	oled.clear()
	oled.flush()
	# Draw blue bars at top and bottom of screen
	n = isnum
	bc = border_color
	y = offset
	for i in range(0, 12):
		oled.fill(0, y + i, 129, y + i+1, [bc.r - (n(bc.r) * i), bc.g - (n(bc.g) * i), bc.b - (n(bc.b) * i)])
		oled.flush()
	y = 116 + offset
	for i in range(0, 12):
		oled.fill(0, y + i, 129, y + i+1, [bc.r - (n(bc.r) * i), bc.g - (n(bc.g) * i), bc.b - (n(bc.b) * i)])
		oled.flush()
	# Show title of menu on the top blue bar that was just drawn
	oled.textbox(x=65, y=4, text=curmenu.name, color=[0, 0, 0], align=2)
	oled.flush()
	# Show current item index and total on bottom blue bar
	oled.textbox(x=65, y=119, text="%s/%s" % (curmenu.index + 1, curmenu.len), color=[0, 0, 0], align=2)
	oled.flush()
	# Redraw the counter (bottom blue bar)
	oled.redraw(116 + offset)
	for y in range(menu_start, curmenu.end, row_height):
		if y == curmenu.cursor:
			# Fill the row of the center menu item because for the selection cursor, iPod-ish style :)
			h = highlight_color
			for i in range(0, 6):
				oled.fill(0, y + i, 129, y + i+1, [h.r - (n(h.r) * i), h.g - (n(h.g) * i), h.b - (n(h.b) * i)])
				oled.flush()
			i2 = 0
			for i in range(6, 11):
				oled.fill(0, y + i, 129, y + i+1, [h.r - (10 * n(h.r)) + i2, h.g - (10 * n(h.r)) + i2, h.b - (10 * n(h.r)) + i2])
				oled.flush()
				i2 += 1
			# Also invert selected item's text color
			oled.textbox (x = 1, y = y + 2, text = curmenu.items[curmenu.top + ((y - menu_start) / row_height)][0], color=texth_color, mode=2)
		else:
			# Plain menu items (but they might get selected later!)
			oled.textbox (x = 1, y = y + 2, text = curmenu.items[curmenu.top + ((y - menu_start) / row_height)][0], color=text_color, mode=2)
		oled.flush()
	#oled.redraw()
	#oled.flush()

# When the down button is pressed
def menu_down ():
	print "Menu Down"
	#If we're in play mode, just put volume down
	if mode == "play":
		audeng.vol_down()
		return
	# Very simple...just increase the menu index to go down the list...
	# ...as long as it isn't already at the limit
	if curmenu.index < curmenu.len - 1:
		curmenu.index += 1
		# Move down the cursor if possible
		if curmenu.cursor < menu_end - row_height:
			curmenu.cursor += row_height
			menu_draw()
			oled.redraw(curmenu.cursor - row_height, curmenu.cursor + row_height)
		else:
			# Move the menu down if the cursor can't
			curmenu.top += 1
			oled.copy(y = menu_start + row_height, y2 = menu_start + row_height * (menu_rows - 1) - 1, y3 = menu_start)
			oled.flush()
			menu_draw()
			oled.redraw(menu_start + (menu_rows - 2) * row_height, menu_start + menu_rows * row_height)
		oled.flush()

# Take a guess (hint: it's when the up butotn is pressed)
def menu_up ():
	print "Menu Up"
	#If we're in play mode, just put volume up
	if mode == "play":
		audeng.vol_up()
		return
	# Equaly as simple as menu_down, just have to decrease the index/cursor instead
	# First, we simply have to check to make sure the index isn't already at 0 (anything lower is invalid)
	if curmenu.index > 0:
		curmenu.index -= 1
		# Move the cursor up if possible
		if curmenu.cursor > menu_start:
			curmenu.cursor -= row_height
			menu_draw()
			oled.redraw(curmenu.cursor, curmenu.cursor + row_height * 2)
		else:
			# If the cursor can't move up, move up the menu
			curmenu.top -= 1
			oled.copy(y = curmenu.cursor + row_height,  y2 = menu_start + row_height * (menu_rows - 1) - 2, y3 = menu_start + row_height * 2)
			oled.flush()
			menu_draw()
			oled.redraw(menu_start, menu_start + row_height * 2)
		oled.flush()

# When action/select button is pressed
def menu_action ():
	global mode
	print "Menu Action"
	# If in play mode, treat action button as play/pause
	if mode == "play":
		audeng.pause()
		return
	# Get code that tells us what's supposed to happen when the item is "clicked"
	action = curmenu.items[curmenu.index][1]
	# Store the current menu in the history, in case we want to go back
	#history.append((curmenu.name, curmenu.items, curmenu.index, curmenu.cursor, curmenu.top))
	# Where all the magic happens (basically causes changes to another menu or starts song playback)
	history.append((curmenu.name, curmenu.items, curmenu.index, curmenu.cursor, curmenu.top))
	if action[:3] == "imd":
		exec(action)
	else:
		# Otherwise, load up the next "dumb" menu
		menu_load(eval("menu." + action))

# Almost made me want to cry at first...but another seemingly stupid var should suffice
# Obviously? occurs when the back button is pressed (to go to previosu menu)
def menu_back ():
	global mode
	print "Menu Back"
	# Get previous and load it, if we're not already home
	if history != []:
		prevmenu = history.pop()
		menu_load([prevmenu[0], prevmenu[1]], prevmenu[2], prevmenu[3], prevmenu[4])
	elif audeng.mode != "stopped":
		imd_playback()
	if mode == "play":
		mode = "browse"

def menu_home ():
	# Semi tricky without this function (we gotta reset the history and load home menu)
	menu_load(menu.home())
	global history
	history = []

def play_prev ():
	global cursong
	if cursong == 0:
		cursong = len(playlist)
	cursong = cursong - 1
	imd_play(playlist[cursong])
	
def play_next ():
	global cursong
	cursong = cursong + 1
	if cursong == len(playlist):
		cursong = 0
	imd_play(playlist[cursong])
	
def isnum (x):
	try:
		if x > 0:
			return 1
		else:
			return 0
	except:
		return 0

def load_color (item, color):
	exec("%s_color.r = %s") % (item, color[0])
	exec("%s_color.g = %s") % (item, color[1])
	exec("%s_color.b = %s") % (item, color[2])

def imd_play (id):
	global cursong
	cursong = id
	song = imd_playback()
	print song.filename
	cursongfile = open("cursong", "w+")
	cursongfile.write("%s" % cursong)
	cursongfile.close()
	audeng.play(song.filename)

# Display playback screen
def imd_playback ():
	global mode
	mode = "play"
	song = menu.imd_db.record(cursong)
	oled.clear()
	oled.flush()
	n = isnum
	bc = border_color
	y = offset
	for i in range(0, 12):
		oled.fill(0, y + i, 129, y + i+1, [bc.r - (n(bc.r) * i), bc.g - (n(bc.g) * i), bc.b - (n(bc.b) * i)])
		oled.flush()
	oled.textbox(x = 65, y = 4, text = "Now Playing", color = black, align = 2)
	oled.flush()
	tcolor = blue
	oled.textbox(x = 65, y = 35, text = song.title, color = tcolor, mode = 1, align = 2)
	oled.flush()
	oled.textbox(x = 65, y = 50, text = song.artist, color = tcolor, mode = 1, align = 2)
	oled.flush()
	oled.textbox(x = 65, y = 65, text = song.album, color = tcolor, mode = 1, align = 2)
	oled.flush()
	oled.redraw()
	oled.flush()
	timer().start()
	return song

def calibrate ():
	i = 1
	while i > -1:
		oled.clear()
		oled.flush()
		oled.textbox(x = 65, y = 50, text = "Calibrating in %s" % i, align = 2)
		oled.flush()
		oled.redraw()
		oled.flush()
		sleep(1)
		i -= 1
	tty.write("c")
	sleep(.4)

class input (threading.Thread):
	def run (self):
		global hold, input_mode
		gpio_binds = ["menu_up()",
					  "menu_action()",
					  "menu_down()",
					  "menu_back()",
					  "play_prev()",
					  "play_next()",
					 ]
		os.system("rm inputfifo")
		os.system("mkfifo inputfifo")
		inputfifo = open("inputfifo", "r")
		
		if input_mode == "accel":
			while 1:
			# Take input from accelerometer (controlled by tilt)
				if tty.inWaiting > 0:
					tty.read(tty.inWaiting())
				try:
					if not hold:
						s = tty.read(6)[2:4]
						if s == "U0":
							menu_up()
							sleep(0.3)
						elif s == "D0":
							menu_down()
							sleep(0.3)
						#elif s == "U1":
						#	menu_up()
						#elif s == "D1":
						#	menu_down()
						elif s =="L0":
							menu_back()
							sleep(0.6)
						elif s == "R0":
							menu_action()
							sleep(0.6)
				except:
					pass
		elif input_mode == "gpio":
			while 1:
			# Take input from GPIOs (buttons)
				try:
					gpio_val = 0
					for i in range(0, 10):
						os.system("cat /proc/gpio/UCB1400-0-%s > inputfifo" % i)
						x = inputfifo.readline()
						if x.find("set"):
							gpio_val += 2**i
					eval(gpio_binds[gpio_val])
				except:
					inputfifo.close()
					pass

class timer (threading.Thread):
	def run (self):
		sleep(.5)
		while 1:
			if mode == "play":
				times = audeng.getpos()
				#print times
				sleep(.8)
				if times:
					percent_elapsed = times[0] * 1.0 / times[1]
					oled.fill(15, 80, 114, 90, blue)
					oled.flush()
					oled.fill(16, 81, 113, 89, black)
					oled.flush()
					
					y = 82
					h = highlight_color
					n = isnum
					#print "bar pos: %s" % (17 + int(95 * percent_elapsed))
					for i in range(0, 6):
						oled.fill(17, y + i, 17 + int(95 * percent_elapsed), y + i+1, [h.r - (n(h.r) * i), h.g - (n(h.g) * i), h.b - (n(h.b) * i)])
						oled.flush()
					i2 = 0
					
					secs = (times[0] % 600) / 10
					min = (times[0] - secs * 10) / 600
					oled.fill(17, 93, 114, 100, black)
					oled.flush()
					oled.textbox(x=17, y=93, text="%s:%s%s" % (min, ("0", "")[secs > 9], secs), color = blue)
					oled.flush()
					time_rem = times[1] - times[0]
					secs_rem = (time_rem % 600) / 10
					min_rem = (time_rem - secs_rem * 10) / 600
					oled.textbox(x=114, y=93, text="-%s:%s%s" % (min_rem, ("0", "")[secs_rem > 9], secs_rem), color = blue, align = 1)
					oled.flush()
					oled.redraw()
					oled.flush()
					if times[0] >= times[1]:
						play_next()
						return
			else:
				return

print "loading colors"
load_color("highlight", green)
load_color("border", pblue)
text_color = blue
# Highlighted text color
texth_color = black
#calibrate()

print "starting up oled and audeng"
os.system("killall oled audeng bplay")
os.system("./oled < /tmp/oledmgr > /dev/null &")
os.system("./audeng | bplay -s 44100 -S -b 16 &")

print "loading home menu"
menu_home()
oled.redraw()
oled.flush()
print "starting threads"
input().start()
print "gui.py done loading"