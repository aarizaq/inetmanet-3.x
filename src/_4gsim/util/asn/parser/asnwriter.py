import asnparser
from asnparser import *

asnobjs = list()
imports = list()
includes = list()

def firstlower(string):
        return string[0].lower() + string[1:]

def checkandhandledeps(asnobj, hdrfile, srcfile):
	retval = 0
	for i in range(0, len(asnobj.objs)):
		obj = asnobj.objs[i]
		if obj.type in types:
			writeobject(obj, hdrfile, srcfile)
		elif obj.type in imports:
                        pass
		else:
			found = 0
			for j in range(0, len(asnobjs)):
				if obj.type == asnobjs[j].name:
					writeobject(asnobjs[j], hdrfile, srcfile)
					found = 1
					break
			if not found:
				print ("Warning: Unknown ASN.1 object type " + obj.type + ". Skipping " + asnobj.name + ".\n")
				retval = -1
	return retval	
			
def writeobject(asnobj, hdrfile, srcfile):
	if asnobj.written == 0 and asnobj.name != '':
		if asnobj.parent != None:
			asnobj.name = asnobj.parent.name + asnobj.name

                # Non standard types

		if asnobj.type not in types:
                        objs = list()
			obj = ASNObject()
			obj.type = asnobj.type
			objs.append(obj)
			asnobj.objs = objs
			#print asnobj.name
			if checkandhandledeps(asnobj, hdrfile, srcfile) != 0:
                                return
			hdrfile.write("typedef " + asnobj.type + " " + asnobj.name + ";\n")
		
		# Null and Boolean
		
		if asnobj.type == "Null" or asnobj.type == "Boolean":
			hdrfile.write("typedef " + asnobj.type + " " + asnobj.name + ";\n")
			asnobj.type = asnobj.name
	
		# Constraint types
		
		if asnobj.type in constrainttypes:
			if asnobj.constrainttype == "CONSTANT":
				hdrfile.write("#define " + asnobj.name + " " + str(asnobj.value) + "\n")
			elif asnobj.constrainttype == "UNCONSTRAINED":
				hdrfile.write("typedef " + asnobj.type + " " + asnobj.name + ";\n")
			else:
				hdrfile.write("typedef " + asnobj.type + "<" + asnobj.constrainttype)
				if asnobj.lowerlimit != '':
					hdrfile.write(", " + asnobj.lowerlimit)
				if asnobj.upperlimit != '':
					hdrfile.write(", " + asnobj.upperlimit)
				hdrfile.write("> " + asnobj.name + ";\n")
			asnobj.type = asnobj.name
		
		# Enumerated
		
		if asnobj.type == "Enumerated":
			asnobj.type = asnobj.name
			hdrfile.write("enum " + asnobj.name + "Values {\n")
			for j in range(0, len(asnobj.objs)):
				childobj = asnobj.objs[j]
				hdrfile.write("\t" + childobj.name + "_" + asnobj.name + " = " + str(j) + ",\n")
			hdrfile.write("};\n")
			hdrfile.write("typedef Enumerated<" + asnobj.constrainttype + ", " + str(len(asnobj.objs) - 1) + "> " + asnobj.name + ";\n")
		
		# Sequence
		
		if asnobj.type == "Sequence":
			optnr = 0
			extnr = 0
			asnobj.type = asnobj.name
			if checkandhandledeps(asnobj, hdrfile, srcfile) != 0:
				return
			hdrfile.write("class " + asnobj.name + " : public Sequence {\n" +
						"private:\n" +
						"\tstatic const void *itemsInfo[" + str(len(asnobj.objs)) + "];\n" +
						"\tstatic bool itemsPres[" + str(len(asnobj.objs)) + "];\n" +
						"public:\n" +
						"\tstatic const Info theInfo;\n"
						"\t" + asnobj.name + "(): Sequence(&theInfo) {}\n\n")
			for j in range(0, len(asnobj.objs)):
                                obj = asnobj.objs[j]
                                hdrfile.write("\tvoid set" + obj.name + "(const " + obj.type + "& " + firstlower(obj.name) + ") { *static_cast<" + obj.type + "*>(items[" + str(j) + "]) = " + firstlower(obj.name) + "; }\n")
			hdrfile.write("};\n")
			srcfile.write("const void *" + asnobj.name + "::itemsInfo[" + str(len(asnobj.objs)) + "] = {\n")
			for j in range(0, len(asnobj.objs)):
				obj = asnobj.objs[j]
				srcfile.write("\t&" + obj.type + "::theInfo,\n")
			srcfile.write("};\n")
			srcfile.write("bool " + asnobj.name + "::itemsPres[" + str(len(asnobj.objs)) + "] = {\n")
			for j in range(0, len(asnobj.objs)):
				obj = asnobj.objs[j]
				if obj.opt == 0:
					srcfile.write("\t1,\n")
				else:
					srcfile.write("\t0,\n")
					optnr += 1
			srcfile.write("};\n")
			srcfile.write("const " + asnobj.name + "::Info " + asnobj.name + "::theInfo = {\n" +
						"\t" + asnobj.name + "::create,\n" +
						"\tSEQUENCE,\n" +
						"\t0,\n")
			if asnobj.constrainttype == "EXTCONSTRAINED":
				srcfile.write("\ttrue,\n")
			else:
				srcfile.write("\tfalse,\n")
			srcfile.write("\titemsInfo,\n" +
						"\titemsPres,\n"
						"\t" + str(len(asnobj.objs)) + ", " + str(optnr) + ", " + str(extnr) + "\n" +
						"};\n")
			srcfile.write("\n")
			
		# Sequence Of
		
		if asnobj.type == "SequenceOf":
			asnobj.type = asnobj.name
			if checkandhandledeps(asnobj, hdrfile, srcfile) != 0:
				return
			hdrfile.write("typedef SequenceOf<" + asnobj.objs[0].type + ", " + asnobj.constrainttype + ", " + asnobj.lowerlimit + ", " + asnobj.upperlimit + "> " + asnobj.name + ";\n")
			
		# Choice
		
		if asnobj.type == "Choice":
			asnobj.type = asnobj.name
			if checkandhandledeps(asnobj, hdrfile, srcfile) != 0:
				return

			hdrfile.write("class " + asnobj.name + " : public Choice {\n" +
						"private:\n" +
						"\tstatic const void *choicesInfo[" + str(len(asnobj.objs)) + "];\n" +
						"public:\n")
			hdrfile.write("\tenum " + asnobj.name + "Choices {\n")
			for j in range(0, len(asnobj.objs)):
                                obj = asnobj.objs[j]
                                hdrfile.write("\t\t" + firstlower(obj.name) + " = " + str(j) + ",\n")
			hdrfile.write("\t};\n")
			hdrfile.write("\tstatic const Info theInfo;\n"
                                                "\t" + asnobj.name + "(): Choice(&theInfo) {}\n")
			hdrfile.write("};\n")
			srcfile.write("const void *" + asnobj.name + "::choicesInfo[" + str(len(asnobj.objs)) + "] = {\n")
			for j in range(0, len(asnobj.objs)):
				obj = asnobj.objs[j]
				srcfile.write("\t&" + obj.type + "::theInfo,\n")
			srcfile.write("};\n")
			srcfile.write("const " + asnobj.name + "::Info " + asnobj.name + "::theInfo = {\n" +
						"\t" + asnobj.name + "::create,\n" +
						"\tCHOICE,\n" +
						"\t0,\n")
			if asnobj.constrainttype == "EXTCONSTRAINED":
				srcfile.write("\ttrue,\n")
			else:
				srcfile.write("\tfalse,\n")
			srcfile.write("\tchoicesInfo,\n" +
						"\t" + str(len(asnobj.objs) - 1) + "\n" +
						"};\n")
                    	srcfile.write("\n")
			
		hdrfile.write("\n")
		asnobj.written = 1

def writeheader(file):
	file.write("//\n" +
		"// Copyright (C) 2012 Calin Cerchez\n" +
		"//\n" +
		"// This program is free software: you can redistribute it and/or modify\n" +
		"// it under the terms of the GNU Lesser General Public License as published by\n" +
		"// the Free Software Foundation, either version 3 of the License, or\n" +
		"// (at your option) any later version.\n" +
		"//\n" + 
		"// This program is distributed in the hope that it will be useful,\n" +
		"// but WITHOUT ANY WARRANTY; without even the implied warranty of\n" +
		"// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" +
		"// GNU Lesser General Public License for more details.\n" +
		"//\n" + 
		"// You should have received a copy of the GNU Lesser General Public License\n" +
		"// along with this program.  If not, see http://www.gnu.org/licenses/.\n" +
		"//\n\n")



def writefile(directory, filename, objs, module):
        global includes
        global imports
        global asnobjs
        asnobjs = objs
        outfilename = asnobjs[0].outfilename
        includes = asnobjs[0].includes
        imports = asnobjs[0].imports
        
        print ("writing source files for " + filename + "...")
	hdrfile = open(directory + outfilename + ".h", 'w')
	srcfile = open(directory + outfilename + ".cc", 'w')
	writeheader(hdrfile)
	writeheader(srcfile)

	hdrfile.write("#ifndef " + outfilename.upper() + "_H_\n" +
			"#define " + outfilename.upper() + "_H_\n\n" +
			"#include \"ASNTypes.h\"\n")
	for i in range(0, len(includes)):
		hdrfile.write("#include \"" + includes[i] + ".h\"\n")
	hdrfile.write("\n")

	srcfile.write("#include \"" + outfilename + ".h\"\n\n")

	srcfile.write("namespace " + module.lower() + " {\n\n")
	hdrfile.write("namespace " + module.lower() + " {\n\n")

	for i in range (0, len(asnobjs)):
		asnobj = asnobjs[i]
		writeobject(asnobj, hdrfile, srcfile)

        srcfile.write("}\n")
        hdrfile.write("}\n\n")

	hdrfile.write("#endif /* " + outfilename.upper() + "_H_ */\n")

	srcfile.close()
	hdrfile.close()

        
