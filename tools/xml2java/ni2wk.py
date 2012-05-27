#!/usr/bin/python

# Niagara2WuKongFlow is a temporary solution that converts the flow of Niagara FBP XML to the WuKong Flow XML
#
# Author: B.L.Tsai
# Date: May 27, 2012

import os
from xml.etree.ElementTree import parse
from optparse import OptionParser
from operator import itemgetter

out_fd = None

def parser():
    parser = OptionParser("usage: %prog [options]")
    parser.add_option("-i", "--input", action="store", type="string", dest="i", metavar="INPUT", help="Input: Niagara FBP XML file path")
    parser.add_option("-o", "--output", action="store", type="string", dest="out", metavar="OUTPUT", help="Output: WuKong Flow XML file path", default="output.xml")
    parser.add_option("-n", "--name", action="append", type="string", dest="nl", metavar="NAMELIST", help='Names of the scenarios "-n A -n B ..."', default=[])
    
    (options, args) = parser.parse_args()
    if not options.i: parser.error("invalid Nigara XML, please refer to -h for help")
    if len(options.nl) == 0: parser.error("invalid Scenario name(s), please refer to -h for help")
    print options.nl

    root = parse(options.i).getroot()
    global out_fd
    out_fd = open(options.out, 'w')

    plist = root.findall(".//p[@t='b:Folder']")
    nlist = [n for n in plist if n.get('n') in options.nl]

    

    return options.out

def codegen():
    print>>out_fd, "HaHa"

if __name__ == "__main__":
    out = parser()
    codegen()
    print "Niagara2WuKongFlow msg: the file %s is generated" % (out)
