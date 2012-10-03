import asnparser
from asnparser import *
import asnwriter
from asnwriter import *
import os
import re

module = "RRC"
#directory = "/root/Desktop/omnetpp-4.2.2/samples/4Gsim/src/linklayer/lte/rrc/message/"
directory = "D:\\omnetpp-4.2.2\\samples\\4Gsim\\src\\linklayer\\lte\\rrc\\message\\"
	
	
##def printobjects(asnobjs):
##	for i in range(0, len(asnobjs)):
##		asnobj = asnobjs[i]
##		print (asnobj.type + " - " + asnobj.name)
##		if asnobj.lowerlimit != '':
##			print ("<" + asnobj.constrainttype + ", " + asnobj.lowerlimit + ", " + asnobj.upperlimit + ">")
##		#for j in range (0, len(asnobj.objs)):
##		#	print asnobj.objs[j]	
##		if len(asnobj.objs) > 0:
##			print ("children s")
##			printobjects(asnobj.objs)
##			print ("children e")


def main():
        os.chdir(directory)
        for filename in os.listdir("."):
                if filename.endswith(".asn"):
                        asnobjs = parsefile(directory, filename, module)
                        writefile(directory, filename, asnobjs, module)
                        
##	usage = "usage: %prog [options] input filename"
##	parser = OptionParser(usage)
##	parser.add_option("-o", "--output", dest="filename",
##		          help="name of output file", metavar="FILENAME")
##
##	(options, args) = parser.parse_args()
##	

##
##	#printobjects(asnobjs)

	
if __name__ == "__main__":
	main()
