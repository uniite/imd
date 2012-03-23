import re, os

width = re.compile("width: (\d+)")

matches = width.findall("".join(open("src/font_arial.h", "r").readlines()))

output = "short font_arial_width[224] = {\n"

for match in matches:
	output += "\t%s,\n" % match

output += "}"

open("font_arial_width.h", "w").write(output)