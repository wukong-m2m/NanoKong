#!/usr/bin/python

# Niagara2WuKongFlow is a temporary solution that converts the flow of Niagara FBP XML to the WuKong Flow XML
#
# Author: B.L.Tsai
# Date: May 27, 2012

import os
import xml.etree.ElementTree as ET
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

    root = ET.parse(options.i).getroot()
    global out_fd
    out_fd = open(options.out, 'w')

    plist = root.findall(".//p[@t='b:Folder']")
    slist = [p for p in plist if p.get('n') in options.nl]
    olist = []

    for scenario in slist:
        clist = scenario.findall("p")
        id2comp = {
            c.get('h'):{'type':c.get('t'), 'iname':c.get('n'), 'link':{},
                        'val':{ i.get('n'):i.get('v') for i in list(set(c.findall("p"))-set(c.findall("p[@t]"))) }
            } for c in clist 
        } # id: name, value_dict, link_dict
        # rename the type, add links to source component in the view of target component
        for component in clist:
            dest = id2comp[component.get('h')]
            dest['type'] = dest['type'][dest['type'].find(':')+1:]
            dname = dest['iname']
            dval = dest['val']
            for i in component.findall("p[@t='b:Link']"):
                srcid = i.find("p[@n='sourceOrd']").get('v')[2:]
                assert srcid in id2comp, "Error! srcid %s is not found under scenario %s" % (srcid, component.get('n'))
                src = id2comp[srcid]
                sprop = i.find("p[@n='sourceSlotName']").get('v')
                dprop = i.find("p[@n='targetSlotName']").get('v')
                sname = src['iname']
                sval = src['val']
                src['link'][sprop] = (dname, dprop)
                # only destination property cannot be set with default value
                if dprop in dval: del dval[dprop]
        olist += [id2comp]

    return options.out, olist

def codegen(scenarios):
    print >> out_fd, 'Ha'

if __name__ == "__main__":
    out, scenarios = parser()
    codegen(scenarios)
    print "Niagara2WuKongFlow msg: the file %s is generated" % (out)
