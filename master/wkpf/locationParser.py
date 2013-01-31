#Sen Zhou 12.8.14 
#define how to parse URLs like EE_Building/3F/ROOM318#near(0,1,2)&inside(3F),  
#(, ), &, |, are preserved key words
#priority | < & < ~
#any kind of spaces, tabs or "\n" are not allowed in URL
import sys, traceback
import os

sys.path.append(os.path.join(os.path.dirname(__file__), "./pyparsing"))
sys.path.append(os.path.join(os.path.dirname(__file__), "./../"))
from configuration import *
from wkpf import WuClass, WuObject, NodeInfo
from locationTree import *
from pyparsing import *
'''
#BNF Rules:
<Number> ::= [0-9] | [0-9] <Number>
<real> ::= <number> |<number> ',' <number>  
<coordinate> ::= ',' <number> ',' <number> ',' <number> ','
<word> ::= '[a-zA-Z_]' | <word> '[a-zA-Z0-9_]'
<function_name> ::=<word>
<parameter> ::= <word> | <real>
<parameter_list> ::= <parameter> | <parameter_list> ',' <parameter>
<function> = <function_name> '('Optional(<parameter_list>) ')'
<opNegate> = <function> | '~' <opNegate>
<opAnd> = <opNegate> | <opNegate> '&' <opAnd>
<opOr> = <opAnd> | <opOr> '|' <opOr>
<path> = '/' <word> | <path> '/' <word> 
<specification> = <path> | <path>'#' <opOr>
<location_def> = <path>'@'<coordinate>
'''
class LocationParser:
    def __init__(self, locationTree):
        self.nodeIdSet = set([]) 
        self.locationTree = locationTree

    def evaluateAnd(self, locTreeNode, argument):
        print "argument in evalAnd:"+str(argument)
        if len(argument)==1:
            return self.evaluate(locTreeNode, argument[0])
        return self.evaluate(locTreeNode, argument[0]).intersection(self.evaluate(locTreeNode, argument[1]))

    def evaluateOr(self, locTreeNode, argument):
        if len(argument)==1:
            return self.evaluate(locTreeNode, argument[0])
        return self.evaluate(argument[0]).union(self.evaluate(locTreeNode, argument[1]))

    def evaluateNegate(self, locTreeNode, argument):
        return locTreeNode.idSet - self.evaluate(locTreeNode, argument[0])
        
    def evaluateFunction(self, locTreeNode, argument):
        arglst = argument[1:]
        print "function arguments:", argument
        return self._funct_dict[argument[0]](locTreeNode, *arglst)
    
    def parsePath(self):
        def evaluatePath(s, loc, toks):
            locTreeNode =  self.locationTree.findLocation(self.locationTree.root, s)
            return [locTreeNode]
        return evaluatePath
        
    def evaluateSpec(self, locTreeNode, argument):
        print "argument: "+str(argument)
        locTreeNode = argument[0]
        if len(argument)<2:     #if no functions after URL, shortcut for URL#getAll()
            return locTreeNode.getAllAliveNodeIds()
        return self.evaluate(locTreeNode, argument[1])
        
    def evaluate(self, locTreeNode, argument):
        print 'name: '+argument.getName()+" "+str(argument)
        return self._funct_dict[argument.getName()](self, locTreeNode, argument)
        
    #different functions
    def isID(locationTreeNode, id):
        id = int(id)
        return set([id])
        
    def getAll(locationTreeNode):
        return locationTreeNode.getAllAliveNodeIds()
        
    def near(locationTreeNode, dist, x, y=None,z=None):
        ret_val = set([])
        dist = float(dist)
        try:            #case one, coordinates are given
            x = float(x)
            y = float(y)
            z = float(z)
        except ValueError:  #case 2, the name of a landmark is given, x would be the landmarkName
            landMarks = locationTreeNode.findLandmarksByName(x)
            # Assume we use the first landmark of the same name
            if landMarks ==None or len(landMarks)==0:   #no such landmark of the name
                return ret_val
            x = landMarks[0].coord[0]
            y = landMarks[0].coord[1]
            z = landMarks[0].coord[2]
        idLst = list(locationTreeNode.idSet)
        for sensorId in idLst:
            sensorNd = locationTreeNode.getSensorById(sensorId)
            if (sensorNd.coord[0]-x)**2+(sensorNd.coord[1]-y)**2+(sensorNd.coord[2]-z)**2 <= dist**2:
                #print sensorNd.coord[0],sensorNd.coord[1],sensorNd.coord[2]
                ret_val.add(sensorNd.nodeInfo.nodeId)
        return ret_val

    #return the nearest 'count' nodes from idset(or location treenode is idset=None) to x,y,z(or x)
    def nearest(locationTreeNode, x, y=None,z=None, count=-1, idLst=None):
        node_lst = []       #list of nodes to be returned
        dist_lst = []       #list of nodes' distances in the node_list
        try:            #case one, coordinates are given
            x = float(x)
            y = float(y)
            z = float(z)
        except ValueError:  #case 2, the name of a landmark is given, x would be the landmarkName
            landMarks = locationTreeNode.findLandmarksByName(x)
            # Assume we use the first landmark of the same name
            if landMarks ==None or len(landMarks)==0:   #no such landmark of the name
                return ret_val
            x = landMarks[0].coord[0]
            y = landMarks[0].coord[1]
            z = landMarks[0].coord[2]

        if count ==-1:
            count = 65535   #count==-1 means we find and sort all

        largest_dist = 0
        if idLst == None:
            idLst = list(locationTreeNode.idSet)
        for sensorId in idLst:
            sensorNd = locationTreeNode.getSensorById(sensorId)
            if sensorNd == None:
                continue
            dist = (sensorNd.coord[0]-x)**2+(sensorNd.coord[1]-y)**2+(sensorNd.coord[2]-z)**2
            if dist >= largest_dist:
                if len(node_lst)<count:
                    #print sensorNd.coord[0],sensorNd.coord[1],sensorNd.coord[2]
                    node_lst.append(sensorNd.nodeInfo.nodeId)
                    dist_lst.append(dist)
                    largest_dist = dist
            else:
                for i in range(len(node_lst)):
                    if dist_lst[i] > dist:
                        node_lst.insert(i, sensorNd.nodeInfo.nodeId)
                        dist_lst.insert(i, dist)
                        break
        return node_lst

    _funct_dict = {
            u"specification":evaluateSpec,u"function":evaluateFunction,
            u"not":evaluateNegate, u"and":evaluateAnd, u"or": evaluateOr, 
            u"near":near, u"isID":isID, u"getAll":getAll, u"nearest": nearest}
            
            
    def _isFuncParameter(self, numStr):
        try:
            numPara = float(numStr)
        except ValueError:
            return False
        return True
    
    def _isFuncName(self, nameStr):
        if self.funct_dict.has_key(nameStr):
            return True
        return False

    def parse(self, str):
        real = Combine( Word(nums) +(u'.' + Word(nums))*(0,1) )
        word = Word( srange(u"[a-zA-Z_]"), srange(u"[a-zA-Z0-9_]") )
        function_name = word
        EQUAL = Literal(u'=').suppress()
        LPAR = Literal(u'(').suppress()
        RPAR = Literal(u')').suppress()
        COMMA = Literal(u',').suppress()
        NOT = Literal(u'~').suppress()
        AND = Literal(u'&').suppress()
        OR  =Literal(u'|').suppress()
        SLASH = Literal(u'/').suppress()
        POND = Literal(u'#').suppress()
        AT = Literal(u'@').suppress()
        parameter = word | real
        parameter_list = parameter + ( COMMA+ parameter )*(None, None)
        function = Group(function_name + LPAR + (parameter_list)*(0,1)+RPAR).setResultsName(u"function")
        coordinate = LPAR + real + (COMMA + real)*2 + RPAR
        opNegate = Forward()        #declaration for recursive definition
        opNegate << (Group(NOT + opNegate).setResultsName(u"not") | function)
        opAnd = Forward()
        opAnd << (Group(opNegate + AND + opAnd).setResultsName(u"and") | Group(opNegate).setResultsName(u"and"))
        opOr = Forward()
        opOr << (Group(opAnd + OR + opOr).setResultsName(u"or") | Group(opAnd).setResultsName(u"or"))
        path = (Group((SLASH + word)*(1, None))).setParseAction(self.parsePath())
        specification = (Group(path + POND + opOr *(0,1))).setResultsName(u"specification") | Group(path).setResultsName(u"specification")
        location_def = path + AT +coordinate   #not used, because this file is for specificaiton parser
        try:
            if str == "" or str == "/":
                str = "/"+ LOCATION_ROOT
            if len(str)>1 and str[0]!='/':
                str = '/'+str
            result =  specification.parseString(str, True)
#            print "parse result: ", result
            return self.evaluate(None, result[0])
        except Exception as e:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print str
            print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                          limit=2, file=sys.stdout)
            raise

if __name__ == "__main__":
    locTree = LocationTree(u"universal")
    loc0 = u"universal/Boli_Building/3F/South_Corridor/Room318@(3,1,8)"
    loc1 = u"universal/Boli_Building/3F/N_Corridor/Room318@(8,1,3)"
    loc2 = u"universal/Boli_Building/2F/South_Corridor/Room318@(4,4,4)"
    loc3 = u"universal/Boli_Building/3F/South_Corridor/Room318@(5,2,2)"
    senNd0 = SensorNode(NodeInfo(0, [], [],loc0))
    senNd1 = SensorNode(NodeInfo(1, [], [], loc1))
    senNd2 = SensorNode( NodeInfo(2, [], [], loc2))
    senNd3 = SensorNode(NodeInfo(3, [], [], loc3))
    locTree.addSensor(senNd0)
    locTree.addSensor(senNd1)
    locTree.addSensor(senNd2)
    locTree.addSensor(senNd3)
    locTree.printTree(locTree.root, 0)
    query = u"/universal/Boli_Building#nearest(4,4,4)"
    query2=u"/universal/Boli_Building#~near(4,4,4,4)&getAll()"
    func = u"nearest(4,4,4)"
    locParser = LocationParser( locTree)
    result = locParser.parse(query)
    print "parser result", result
    print [locTree.getNodeInfoById(nodeId) for nodeId in result]
#    print locParser.evaluate(None, locParser.parse(query2)[0])
#   print locURLHandler.locationTreeNode.sensorLst
    loc3 = u"universal/Boli_Building/3F/South_Corridor/Room318#getAll()"
#res = LocationParser.loc_def.parseString(loc3)
#print res
   #