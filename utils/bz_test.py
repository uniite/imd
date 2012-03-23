from time import time
from buzhug import Base
from os import system

start = time()

db=Base('t1').create(('a',int),('b',int),('c',str))
for i in range(0, 1000):
    db.insert(i, i, "stuff%s" % i)
db.commit()
print "Finished: %ss" % (time() - start)
system("pause")