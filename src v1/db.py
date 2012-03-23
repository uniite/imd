import os, sys, MP3Info, struct
from buzhug import Base

mp3info = MP3Info.MP3Info
db = Base("/root/mymusic")

# Decodes a tag from MP3Info into a string (helps in rare cases)
def dectag (data):
    try:
        if data.find("\x00") == -1:
            return data
        else:
            # One of those rare cases..
            data = data.split("\x00")
            data[0] = data[0][-1]
            return "".join(data)
    except:
        return ''

def create ():
    db.create (
                ('title', str),
                ('artist', str),
                ('album', str),
                ('genre', str),
                ('filename', str),
                ('mdate', str)
                )
    
def opendb ():
    db.open()

def close ():
    db.cleanup()

def addfile (file):
    # First lets get the ID3 tags
    try:
        tags = mp3info(open(file, "rb"))
        db.insert (
                    dectag(tags.title),
                    dectag(tags.artist),
                    dectag(tags.album),
                    dectag(tags.genre),
                    file,
                    str(os.path.getmtime(file))
                    )
    except:
        # Problem getting ID3 Tags
        print "Could not add " + file
        db.insert(title = file.split("/")[-1], mdate = str(os.path.getmtime(file)))

def updatefile (file):
    db.select_for_update(file)
    # First lets get the ID3 tags
    try:
        tags = mp3info(open(file, "rb"))
        db.update (
                dectag(tags.title),
                dectag(tags.artist),
                dectag(tags.album),
                dectag(tags.genre),
                mdate = str(os.path.getmtime(file))
                ) 
    except:
        db.update (file.split("/")[-1], mdate = str(os.path.getmtime(file)))

# Should be used with /mnt/mmc/music to build music db
def scanfiles (path):
    os.path.walk(path, scanfiles2, "")

def scanfiles2 (arg, dir, files):
    for file in files:
        temp = db.select(['mdate'], filename = dir + "/" + file)
        if len(temp) > 0:
            # Check modification date
            if os.path.getmtime(dir + "/" + file) > int(temp[0].mdate):
                # Update file because it has been changed
                updatefile(dir + "/" + file)
        else:
            addfile(dir + "/" + file)

# Used to check for and remove files from the databse that no longer exist
def clean ():
	files = db.select(['filename'])
	for file in files:
		if not os.path.isfile(file.filename):
			db.delete(file)
