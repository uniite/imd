import re

p = re.compile("(0b)")

def convert(path):
    file = open(path, "r+")
    lines = file.readlines()
    file.close()
    for i in range(len(lines)):
        find = p.search(lines[i])
        if find != None:
            lines[i] = lines[i][:find.start()-1] + hex(bintoint(lines[i][find.start()+2:find.start()+10])) + lines[i][find.start()+10:]
    file = open(path + "-converted", "w")
    file.writelines(lines)
    file.close()

def bintoint(bin):
     int = 0
     for i in range(0,len(bin)):
             if bin[i] == '1':
                    int += 2**i
     return int

#bintoint.convert("/home/notroot/oled/fonts")