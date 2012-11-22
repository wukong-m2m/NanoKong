#!/usr/bin/python
# -*- coding: utf-8 -*-

import glob
import os

def addHead():
	g = open("./templates/fbp.html","a")
	g.write('\n'+
            '<html>'+'\n'+
            '\t'+'<head>'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/smoothness/jquery-ui-1.8.21.custom.css" rel="stylesheet" />'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/jquery.contextMenu.css" rel="stylesheet" />'+'\n'+
            '\t\t'+'<link type="text/css" href="/static/css/fbp.css" rel="stylesheet" />'+'\n'+
            '\t\t'+'<script type="text/javascript" src="/static/js/jquery-1.7.2.min.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/jquery-ui-1.8.21.custom.min.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/jquery.contextMenu.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/jcanvas.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/json2.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/block.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/line.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/fbp.js"></script>'+'\n'+
			'\t\t'+'<script type="text/javascript" src="/static/js/flowlist.js"></script>'+'\n'
            )
	g.close()

def addMiddle(aList):
    for list in aList:
    	list = list.replace('./','/')
        g = open("./templates/fbp.html","a")
        g.write('\t\t'+'<script type="text/javascript" src="'+list+'"></script>'+'\n')
        g.close()

def addEnd():
    g = open("./templates/fbp.html","a")
    g.write(
                '\t'+'</head>'+'\n'+
                '\t'+'<body scrolling="no">'+'\n'+
                '\t\t'+'<canvas id=canvas style="position:absolute; left:100; top:50;" height=600 width=800></canvas>'+'\n'+
                '\t\t'+'<div id=client></div>'+'\n'+
                '\t\t'+'<canvas id=canvastop style="position:absolute; left:0; top:50;" height=600 width=800></canvas>'+'\n'+
                '\t\t'+'<div id=connection>'+'\n'+
                '\t\t'+'<table>'+'\n'+
				'\t\t'+'<tr><td>From</td></tr>'+'\n'+
				'\t\t'+'<tr>'+'\n'+
                '\t\t'+'<td><select id=connection_src></select></td>'+'\n'+
				'\t\t'+'</tr>'+'\n'+
				'\t\t'+'<tr><td>To</td></tr>'+'\n'+
				'\t\t'+'<tr>'+'\n'+
                '\t\t'+'<td><select id=connection_act></select></td>'+'\n'+
				'\t\t'+'</tr>'+'\n'+
                '\t\t'+'</table>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div id=msg style="position:absolute;background-color:#ffff00"></div>'+'\n'+
                '\t\t'+'<div id=fileloader>'+'\n'+
                '\t\t'+'<input type=text id=fileloader_file></input>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div id=propertyeditor>'+'\n'+
                '\t\t'+'<div> Location </div>'+'\n'+
                '\t\t'+'<input type=text id=propertyeditor_location></input>'+'\n'+
                '\t\t'+'</div>'+'\n'+
                '\t\t'+'<div id="progress"><div id="compile_status"></div>'+'\n'+
                '\t\t'+'<div id="normal"></div><div id="critical_error"></div>'+'\n'+
                '\t\t'+'<div id="urgent_error"></div></div>'+'\n'+
                '\t'+'</body>'+'\n'+
                '</html>'
                )
    g.close()


 
def check(name):
	isFile = os.path.exists(name)
	return isFile 

class fbp_main:
	def __init__(self):
		print ""
		
	def make(self):
		existFile = os.path.exists("./templates/fbp.html")
		if existFile:
		    os.remove("./templates/fbp.html")

		addHead()
#		_jsList = glob.glob("./static/js/*.js")
		_testList = glob.glob("./static/js/test/*.js")
#		addMiddle(_jsList)
		addMiddle(_testList)
		addEnd()

		print "make_fbp_complete" 
