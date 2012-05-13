from xml.dom.minidom import parse
from optparse import OptionParser

comp_dom = 0
flow_dom = 0


def parser():
    parser = OptionParser("usage: %prog [options] arg")
    parser.add_option("-c", "--component", action="store", type="string", dest="pathc", help="WuKong Component XML file path")
    parser.add_option("-f", "--flow", action="store", type="string", dest="pathf", help="WuKong Flow XML file path")
    (options, args) = parser.parse_args()
    if not options.pathc or not options.pathf:
        parser.error("invalid component and flow xml, please refer to -h for help")
    global comp_dom, flow_dom
    comp_dom = parse(options.pathc)
    flow_dom = parse(options.pathf)



if __name__ == "__main__":
    parser()

