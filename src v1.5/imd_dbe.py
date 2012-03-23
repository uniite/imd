# IMD DB Engine v1
# Created Dec 7, 2007, 7:30pm
# by Jon Botelho
# First working version: Dec 8, 2007, 10:25pm
# 12/9: added getrecs (works better for menu.py)
# 12/15: ported to C for v2
print "importing os"
import os
print "importing binascii"
import binascii
import struct

print "imd_dbe.py done importing"

db_open = 0
db_name = "musicdb"
tfields = ["title", "artist", "album", "genre"]      # Text fields (64byte)
nfields = ["mdate", "adate", "cdate", "duration"]    # Number fields (4byte)
class record_t:
    pass

db_data = None
db_querys = None
db_idpool = None
idpool = None
title_vals = {}
artist_vals = {}
album_vals = {}
genre_vals = {}
filename_vals = {}

print "imd_dbe.py done loading"

# Create database
def create ():
    global db_data, db_querys, db_idpool, db_open, db_name
    
    if db_open:
        close()
    try:
        os.system("mkdir %s" % db_name)
        for filename in ["data", "querys", "idpool"]:
            eval("open('%s/%s', 'w').close()" % (db_name, filename))
        
        db_idpool = open("%s/idpool" % db_name, "w")
        db_idpool.write("\0" * 4)
        db_idpool.close()
        
        db_filename = db_name
        return 1
    except:
        return 0

# Open database
def opendb ():
    global db_data, db_querys, db_idpool, db_open, db_name, idpool, db_name
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals
    idpool = []
    
    for filename in ["data", "querys", "idpool"]:
        globals()["db_%s" % filename] = open("%s/%s" % (db_name, filename), "r+")

    temp = db_querys.readlines()
    for i in range(0, len(temp), 2):
        x = temp[i].find(":")
        field = temp[i][:x]
        val = temp[i][x + 1:-1]
        if globals().has_key("%s_vals" % field):
            globals()["%s_vals" % field][val] = eval(temp[i + 1][:-1])
        
    
    id_count = os.fstat(db_idpool.fileno())[6] / 4
    
    for id in range(0, id_count):
        idpool.append(int(binascii.hexlify(db_idpool.read(4)), 16))
    
    db_idpool.close()
    
    # Allows IDs to just be popped, starting with the lower ones
    idpool.sort()
    idpool.reverse()

    db_open = 1

# Close database
def close ():
    if not db_open:
        return None
        
    for filename in ["data", "querys", "idpool"]:
            globals()["db_%s" % filename].close()

# Commits (saves) database to the files (on disk/flash)
#   *basically we need to handle everything except 
#    the actual record data
def commit ():
    global db_querys, db_idpool, db_name, idpool
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals

    buff = ""   # Write buffer
    
    
    #First lets save the query index
    try:
        db_querys.close()
    except:
        pass
    # Reopen it in write mode to overwrite everything
    db_querys = open("%s/querys" % db_name, "w")
    for field in tfields:
        fdict = globals()["%s_vals" % field]
        for val, ids in fdict.items():
            buff += "%s:%s\n" % (field, val)
            buff += ("%s\n" % ids).replace(", ", ",")
        db_querys.write(buff)
        buff = ""
    field = "filename"
    for val, ids in filename_vals.items():
        buff += "%s:%s\n" % (field, val)
        buff += ("%s\n" % ids).replace(", ", ",")
    db_querys.write(buff)
    buff = ""
    db_querys.close()
    
    # Now for the id pool
    try:
        db_idpool.close()
    except:
        pass
    db_idpool = open("%s/idpool" % db_name, "w")
    for id in idpool:
        buff += binascii.unhexlify(hex(id)[2:].rjust(8, "0"))
    db_idpool.write(buff)
    db_idpool.close()
    buff = ""
    

# Rebuilds query index
def rebuild ():
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals
    
    for field in tfields:
        globals()["%s_vals" % field] = {}   # Create {field}_vals set for each field
    for id in range(0, idpool[0]):          # Loop through all records
        db_data.seek(id * 512 + 7)
        if db_data.read(1) != "\xff":       # Make sure record is valid
            db_data.seek(id * 512 + 11)
            for field in tfields:           # Add values to index
                val = db_data.read(64).strip()
                fdict = globals()["%s_vals" % field]
                if fdict.has_key(val):
                    fdict[val].append(id)
                else:
                    fdict[val] = [id]

# Add data
def insert (title = "", artist = "", album = "", genre = "", filename = "",
            mdate = 0, adate = 0, cdate = 0, duration = 0):
            
    global db_data, db_querys, db_idpool, db_open, db_name, idpool
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals
    
    if not db_open:
        print "db not open"
        return None
    
    if filename_vals.has_key(filename):
        print "record already exists for filename"
        return None
    
    id = idpool.pop()
    if len(idpool) == 0:
        idpool.append(id + 1)
	
    rec = "(dbrec("
    rec += struct.pack("I", id)
    rec += title[0:64].ljust(64)
    rec += artist[0:64].ljust(64)
    rec += album[0:64].ljust(64)
    rec += genre[0:64].ljust(64)
    rec += filename.ljust(200)
    rec += struct.pack ("IIII",
                        mdate,
                        adate,
                        cdate,
                        duration
                        )
    rec += " " * 20 + ")dbrec)"
    
    db_data.seek(id * 512)
    db_data.write(rec)
    
    for field in tfields:   # Add item to index
        fdict = globals()["%s_vals" % field]
        val = eval(field)
        if fdict.has_key(val):
            fdict[val].append(id)
        else:
            fdict[val] = [id]
    
    filename_vals[filename] = [id]
    
    db_data.flush()
    
    return id

# Returns a record class (like a struct) with the record's data
def record (id):
    global db_data, db_querys, db_idpool, db_open, db_name, idpool, tfields
    
    if not db_open:
        return None
    
    if id >= idpool[0]:
        return None
    
    db_data.seek(id * 512 + 7)
    if db_data.read(1) == "\xff":
        return None
    
    rec = record_t()
    rec.id = id
    db_data.seek(id * 512 + 11)
    for field in tfields:
        val = db_data.read(64).strip()
        exec("rec.%s = val" % field)
    rec.filename = db_data.read(200).strip()
    for field in nfields:
        val = int(binascii.hexlify(db_data.read(4)), 16)
        exec("rec.%s = val" % field)
    
    return rec

# Returns a list of IDs that match the query (really fast since the query index is in RAM)
def select (title = None, artist = None, album = None, genre = None, filename = None):
    global db_data, db_querys, db_idpool, db_open, db_name, idpool
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals
    rdict = {}  # Dictionary used to filter results
    
    if not db_open:
        return None
    
    filters = 0
    
    if filename and filename_vals.has_key(filename):
            rdict[filename_vals[filename][0]] = 1
            filters += 1
    
    for field in tfields:
        fdict = globals()["%s_vals" % field]
        val = eval(field)
        if val != None:
            filters += 1
            if fdict.has_key(val):
                ids = fdict[val]
                for id in ids:
                    if rdict.has_key(id):
                        rdict[id] += 1
                    else:
                        rdict[id] = 1
            else:
                return []
    
    if filters == 0: # If all records needed...
        result = range(0, idpool[0] + 1)
        for id in idpool:
            result.remove(id)
        return result
    
    result = []
    for id, matches in rdict.items():
        if matches == filters:
            result.append(id)
    
    return result

# Works the way select usually does in other engines: it retrieves the actual data from the records instead of just IDs
def getrecs (title = None, artist = None, album = None, genre = None, filename = None):
    ids = select(title, artist, album, genre, filename)
    recs = []
    for id in ids:
        recs.append(record(id))
    
    return recs
    
# Update a record
def update (id, title = None, artist = None, album = None, genre = None, filename = None,
            mdate = None, adate = None, cdate = None, duration = None):
    
    global db_data, db_querys, db_idpool, db_open, db_name, idpool
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals
    
    if not db_open:
        return None
    
    if id < idpool[0]:
        db_data.seek(id * 512 + 7)
        if db_data.read(1) == "\xff":
            return None
    
    pos = id * 512 + 11
    
    for field in tfields:
        f = eval(field)
        if f != None:
            db_data.seek(pos)
            db_data.write(f[0:64].ljust(64))
        pos += 64
    
    if filename == None or len(filename > 200):
        return None
    else:
        db_data.seek(pos)
        db_data.write(filename.ljust(200))
    pos += 200
    
    for field in nfields:
        n = eval(field)
        if n != None and n < 2**32:
            db_data.write(binascii.unhexlify(hex(n)[2:].rjust(8, "0")))
        pos += 4

    db_data.flush()
    
    return id

def delete (id):
    global tfields, title_vals, artist_vals, album_vals, genre_vals, filename_vals
    
    if not db_open:
        return None
        
    if id < 2**31:
        db_data.seek(id * 512 + 7)
        db_data.write("\xff")
        db_data.seek(id * 512 + 11)
        for field in tfields:   # Delete item from index
            val = db_data.read(64).strip()
            fdict = globals()["%s_vals" % field]
            if fdict.has_key(val):
                try:
                    fdict[val].remove(val)
                except:
                    pass
                if fdict[val] == []:
                    del(fdict[val])
        if filename_vals.has_key(db_data.read(200).strip()):
            del(filename_vals[filename])
        idpool.append(id)
        idpool.sort()
        idpool.reverse()
        commit()
