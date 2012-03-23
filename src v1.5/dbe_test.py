from time import time
from imd_dbe import *
from os import system

start = time()

create()
opendb()
for i in range(0, 1000):
    insert(mdate = i, duration = i, filename=("lolz%s" % i))
commit()
print "Finished: %ss" % (time() - start)
system("pause")