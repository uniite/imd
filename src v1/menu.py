# Modified 12/9 for use with new imd_dbe

print "importing db"
import db as imd_db
#Songs > : imd_menu_goto("songs")
#Artists > : imd_menu_goto("artists")
#Albums > : imd_menu_goto("albums")
#Genres > : imd_menu_goto("genres")
#Settings > : imd_goto_menu("settings")
print "opening db"
imd_db.opendb()

print "menu.py done loading"

def home ():
	# Main "Home" Menu
	return ["Home", [("Songs", "songs()"), ("Artists", "artists()"), ("Albums", "albums()"), ("Genres", "genres()")]]

def artists (genre = None):
	if genre == None:
		temp = imd_db.getrecs()
		items = set()
	else:
		temp = imd_db.getrecs(genre = genre)
		items = set([("All Songs", "songs(genre = '%s')" % genre )])
	for item in temp:
		items.add((item.artist, "albums(artist = '%s')" % item.artist))
	# Return sorted list...same as all the others
	items = list(items)
	items.sort()
	return ["Artists", items]
	
def albums (artist = None, genre = None):
	if artist != None:
		temp = imd_db.getrecs(artist = artist)
		header = ("All Songs", "songs(artist = '%s')" % artist)
		items = set()
	elif genre != None:
		header = None
		temp = imd_db.getrecs(genre = genre)
		items = set()
		for item in temp:
			items.add((item.album, "songs(album = '%s', genre = '%s')" % (item.album, item.genre)))
	else:
		temp = imd_db.getrecs()
		header = None
		items = set()
	for item in temp:
		if len(item.album.strip()) > 0:
			items.add((item.album, "songs(album = '%s')" % item.album))
	items = list(items)
	items.sort()
	if header:
		items.insert(0, header)
	return ["Albums", items]

def genres (album = None):
	if album == None:
		temp = imd_db.getrecs()
		items = set()
	else:
		temp = imd_db.getrecs(album = album)
		items = set([("All Songs", "songs(genre = '%s')" % genre )])
	for item in temp:
		items.add((item.genre, "albums(genre = '%s')" % item.genre))
	items = list(items)
	items.sort()
	return ["Genres", items]

def songs (artist = None, album = None, genre = None):
	query = "imd_db.getrecs("
	if artist:
		query += "artist = artist, "
	if album:
		query += "album = album, "
	if genre:
		query += "genre = genre"
	query += ")"
	temp = eval(query)
	items = []
	for item in temp:
		items.append((item.title, "imd_play(id = %s)" % item.id))
	items = list(items)
	items.sort()
	return ["Songs", items]