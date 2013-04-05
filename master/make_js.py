#SatoKatsunori made Oct.16 2012 

import os
import logging
from wkpf.configuration import *

lst = [];

def addHead(name):
	head_path = os.path.join(ROOT_PATH, "master/static/js/__comp__"+ name.lower()+".js")
	g = open(head_path, "a")
	g.write("// vim: ts=4 sw=4"+"\n"+
          	"function "+ name +"()"+"\n"+
			"{"+"\n"+
			"\t"+"Block.prototype.init.call(this);"+"\n"+
			"\t"+"this.type='"+name+"';"+"\n"
	)
	g.close()

def addSignal(name, signal):
	signal_path = os.path.join(ROOT_PATH, "master/static/js/__comp__"+ name.lower()+".js")
	g = open(signal_path, "a")
#	g.write("\t"+"this.addSignal( new Signal('"+ signal + "','"+ _type +"'));"+"\n")
	g.write("\t"+"this.addSignal( new Signal('"+ signal + "'));"+"\n")
	g.close()

def addAction(name, action):
	action_path = os.path.join(ROOT_PATH, "master/static/js/__comp__"+ name.lower()+".js")
	g = open(action_path, "a")
#	g.write("\t"+"this.addAction( new Action('"+ action + "','"+ _type +"'));"+"\n")
	g.write("\t"+"this.addAction( new Action('"+ action + "'));"+"\n")
	g.close()

def addEnd(name):
	end_path = os.path.join(ROOT_PATH, "master/static/js/__comp__"+ name.lower()+".js")
	g = open(end_path, "a")
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
    dataType = getName(list, "datatype")
    kind = getName(list,"access")
    if kind == "readonly":
        addSignal(fileName,propertyName)
    elif kind == "writeonly":
        addAction(fileName,propertyName)
    elif kind == "readwrite":
        addAction(fileName,propertyName)
        addSignal(fileName,propertyName)
        
def check(name):
	isFile = os.path.exists("./static/js/__comp__"+ name.lower()+".js")
	return isFile
        
class make_main:
	def make(self):
		filename = COMPONENTXML_PATH
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
		logging.info("make_js_complete")
