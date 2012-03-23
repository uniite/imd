# IMD db version 1.5
#2007
# Dec 8: modified to work with imd_dbe instead of buzhug (hence v1.5)
print "importing os"
import os
print "importing sys"
import sys
print "importing MP3Info"
import MP3Info
print "importing imd_dbe"
from imd_dbe import *

mp3info = MP3Info.MP3Info
print "openign database"

print "db.py done loading"

# Decodes a tag from MP3Info into a string (helps in rare cases)
def dectag (data):
    # No error chekcing needed here; add/updatefile takes care of it
    if data == None:
        return ""
    if data.find("\x00") == -1:
        return data
    else:
        # One of those rare cases..
        data = data.split("\x00")
        data[0] = data[0][-1]
        return "".join(data)

def addfile (file):
    print "Adding: ", file
    # First lets get the ID3 tags
    if 1:
        tags = mp3info(open(file, "rb"))
        if tags.title:
            insert (
                    dectag(tags.title),
                    dectag(tags.artist),
                    dectag(tags.album),
                    dectag(tags.genre),
                    file,
                    os.path.getmtime(file),
                    os.path.getatime(file),
                    os.path.getctime(file)
                    )
        else:
            # Problem getting ID3 Tags or adding file
            print "Could not add " + file
            insert (
                    file.split("/")[-1],
                    filename = file, 
                    mdate = os.path.getmtime(file),
                    adate = os.path.getatime(file),
                    cdate = os.path.getctime(file)
                    )
    commit()

def updatefile (file):
    id = select(filename = file)[0]
    print "Updating: ", file
    try:
        # First lets get the ID3 tags
        tags = mp3info(open(file, "rb"))
        # Now update the record
        update (
                id,
                dectag(tags.title),
                dectag(tags.artist),
                dectag(tags.album),
                dectag(tags.genre),
                mdate = str(os.path.getmtime(file))
                ) 
    except:
        # Something probably went wrong with reading the ID3 tags, so go on without them
        update (
                id,
                file.split("/")[-1],
                filename = file, 
                mdate = os.path.getmtime(file),
                adate = os.path.getatime(file),
                cdate = os.path.getctime(file)
                )
    commit()

# Should be used with /mnt/mmc/music to build music db
def scanfiles (path):
    os.path.walk(path, scanfiles2, "")

def scanfiles2 (arg, dir, files):
    for file in files:
        temp = select(filename = dir + "/" + file)
        if len(temp) > 0:
            # Check modification date
            if os.path.getmtime(dir + "/" + file) > int(record(temp[0]).mdate):
                # Update file because it has been changed
                updatefile(dir + "/" + file)
        else:
            addfile(dir + "/" + file)

# Used to check for and remove files from the databse that no longer exist
def clean ():
    files = select()
    for file in files:
        if not os.path.isfile(record(file).filename):
            delete(file)
    commit()
