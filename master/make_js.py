#SatoKatsunori made Oct.16 2012 

import os

lst = [];

def addHead(name):
	g = open("./static/js/test/__comp__"+ name.lower()+".js","a")
	g.write("// vim: ts=4 sw=4"+"\n"+
          	"function "+ name +"()"+"\n"+
			"{"+"\n"+
			"\t"+"Block.prototype.init.call(this);"+"\n"+
			"\t"+"this.type='"+name+"';"+"\n"
	)
	g.close()

def addSignal(name, signal):
	g = open("./static/js/test/__comp__"+ name.lower()+".js","a")
	g.write("\t"+"this.addSignal( new Signal('"+ signal + "'));"+"\n")
	g.close()

def addAction(name, action):
	g = open("./static/js/test/__comp__"+ name.lower()+".js","a")
	g.write("\t"+"this.addAction( new Action('"+ action + "'));"+"\n")
	g.close()

def addEnd(name):
	g = open("./static/js/test/__comp__"+ name.lower()+".js","a")
	g.write("}"+"\n"+"\n"+"\n"+
			name +".prototype = new Block();"+"\n"+
			name +".prototype.constructor = "+ name +";\n"+
			"Block.register('"+ name +"',"+ name +");"
	)
	g.close()

def getName(list,_name):
    flag = False
    for aList in list:
        if flag:
            return aList
        else:
            if _name in aList:
                flag = True

def getPropertyName(list,fileName):
    propertyName = getName(list,"name")
    kind = getName(list,"access")
    if kind == "readonly":
        addSignal(fileName,propertyName)
    elif kind == "writeonly":
        addAction(fileName,propertyName)
    elif kind == "readwrite":
        addAction(fileName,propertyName)
        addSignal(fileName,propertyName)
        
def check(name):
	isFile = os.path.exists("./static/js/test/"+ name.lower()+".js")
	return isFile
        
class make_main:
	def __init__(self):
		print ""
		
	def make(self):
		filename = "../ComponentDefinitions/WuKongStandardLibrary.xml"
		f = open(filename,"r")
		fileName = ""
		existFile = False
		for row in f:
			list_ = row.split('"')
			for aList in list_:
				if "<WuClass" in aList:
					fileName = getName(list_,"name")
					existFile = check(fileName)
					lst.append(fileName.lower())
					if not existFile:
						addHead(fileName)
					else:
						break
			  	elif "property" in aList and not existFile:
				  	getPropertyName(list_,fileName)
					break
				elif "</WuClass>" in aList and not existFile:
				  	addEnd(fileName)
					name = ""
					break
				else:
					pass
		print "make_js_complete"
