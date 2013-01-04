#Sen Zhou 12.8.14 
#define how to parse URLs like EE_Building/3F/ROOM318#near(0,1,2)&inside(3F),  
#(, ), &, |, are preserved key words
#priority | < & < ~
#any kind of spaces, tabs or "\n" are not allowed in URL
import sys, traceback
import os
sys.path.append(os.path.join(os.path.dirname(__file__), "./pyparsing"))
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
<path> = Optional('/') <word> | <path> '/' <word> 
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
            landMarks = locationTreeNode.findLandMarksByName(x)
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
            landMarks = locationTreeNode.findLandMarksByName(x)
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
            result =  specification.parseString(str, True)
            print "parse result: ", result
            return self.evaluate(None, result[0])
        except:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            print traceback.print_exception(exc_type, exc_value, exc_traceback,
                                          limit=2, file=sys.stdout)

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
    query = u"/universal/Boli_Building"
    query2=u"/universal/Boli_Building#~near(4,4,4,4)&getAll()"
    func = u"near(4,4,4,4)&getAll()"
    locParser = LocationParser( locTree)
    print "parser result", locParser.parse(query)
#    print locParser.evaluate(None, locParser.parse(query2)[0])
#   print locURLHandler.locationTreeNode.sensorLst
    loc3 = u"universal/Boli_Building/3F/South_Corridor/Room318#getAll()"
#res = LocationParser.loc_def.parseString(loc3)
#print res
   #
# simpleBool.py
#
# Example of defining a boolean logic parser using
# the operatorGrammar helper method in pyparsing.
#
# In this example, parse actions associated with each
# operator expression will "compile" the expression
# into BoolOperand subclass objects, which can then
# later be evaluted for their boolean value.
#
# Copyright 2006, by Paul McGuire
#
'''
class SearchQueryParser:

    def __init__(self):
        self._methods = {
            'and': self.evaluateAnd,
            'or': self.evaluateOr,
            'not': self.evaluateNot,
            'parenthesis': self.evaluateParenthesis,
            'quotes': self.evaluateQuotes,
            'word': self.evaluateWord,
            'wordwildcard': self.evaluateWordWildcard,
        }
        self._parser = self.parser()
    
    def parser(self):
        """
        This function returns a parser.
        The grammar should be like most full text search engines (Google, Tsearch, Lucene).
        
        Grammar:
        - a query consists of alphanumeric words, with an optional '*' wildcard
          at the end of a word
        - a sequence of words between quotes is a literal string
        - words can be used together by using operators ('and' or 'or')
        - words with operators can be grouped with parenthesis
        - a word or group of words can be preceded by a 'not' operator
        - the 'and' operator precedes an 'or' operator
        - if an operator is missing, use an 'and' operator
        """
        operatorOr = Forward()
        
        operatorWord = Group(Combine(Word(alphanums) + Suppress('*'))).setResultsName('wordwildcard') | \
                            Group(Word(alphanums)).setResultsName('word')
        
        operatorQuotesContent = Forward()
        operatorQuotesContent << (
            (operatorWord + operatorQuotesContent) | operatorWord
        )
        
        operatorQuotes = Group(
            Suppress('"') + operatorQuotesContent + Suppress('"')
        ).setResultsName("quotes") | operatorWord
        
        operatorParenthesis = Group(
            (Suppress("(") + operatorOr + Suppress(")"))
        ).setResultsName("parenthesis") | operatorQuotes

        operatorNot = Forward()
        operatorNot << (Group(
            Suppress(Keyword("not", caseless=True)) + operatorNot
        ).setResultsName("not") | operatorParenthesis)

        operatorAnd = Forward()
        operatorAnd << (Group(
            operatorNot + Suppress(Keyword("and", caseless=True)) + operatorAnd
        ).setResultsName("and") | Group(
            operatorNot + OneOrMore(~oneOf("and or") + operatorAnd)
        ).setResultsName("and") | operatorNot)
        
        operatorOr << (Group(
            operatorAnd + Suppress(Keyword("or", caseless=True)) + operatorOr
        ).setResultsName("or") | operatorAnd)

        return operatorOr.parseString

    def evaluateAnd(self, argument):
        print argument
        return self.evaluate(argument[0]).intersection(self.evaluate(argument[1]))

    def evaluateOr(self, argument):
        return self.evaluate(argument[0]).union(self.evaluate(argument[1]))

    def evaluateNot(self, argument):
        return self.GetNot(self.evaluate(argument[0]))

    def evaluateParenthesis(self, argument):
        return self.evaluate(argument[0])

    def evaluateQuotes(self, argument):
        """Evaluate quoted strings

        First is does an 'and' on the indidual search terms, then it asks the
        function GetQuoted to only return the subset of ID's that contain the
        literal string.
        """
        r = set()
        search_terms = []
        for item in argument:
            search_terms.append(item[0])
            if len(r) == 0:
                r = self.evaluate(item)
            else:
                r = r.intersection(self.evaluate(item))
        return self.GetQuotes(' '.join(search_terms), r)

    def evaluateWord(self, argument):
        return self.GetWord(argument[0])

    def evaluateWordWildcard(self, argument):
        return self.GetWordWildcard(argument[0])
        
    def evaluate(self, argument):
        print 'name'+argument.getName()
        return self._methods[argument.getName()](argument)

    def Parse(self, query):
        print 'name'+self._parser(query)[0].getName()
        return self.evaluate(self._parser(query)[0])

    def GetWord(self, word):
        return set()

    def GetWordWildcard(self, word):
        return set()

    def GetQuotes(self, search_string, tmp_result):
        return set()

    def GetNot(self, not_set):
        return set().difference(not_set)


class ParserTest(SearchQueryParser):
    """Tests the parser with some search queries
    tests containts a dictionary with tests and expected results.
    """
    tests = {
        'help': set([1, 2, 4, 5]),
 #      'help or hulp': set([1, 2, 3, 4, 5]),
  #      'help and hulp': set([2]),
#        'help hulp': set([2]),
 #       'help and hulp or hilp': set([2, 3, 4]),
        'help or hulp and hilp': set([1, 2, 3, 4, 5]),
#        'help or hulp or hilp or halp': set([1, 2, 3, 4, 5, 6]),
 #       '(help or hulp) and (hilp or halp)': set([3, 4, 5]),
  #      'help and (hilp or halp)': set([4, 5]),
   #     '(help and (hilp or halp)) or hulp': set([2, 3, 4, 5]),
   #     'not help': set([3, 6, 7, 8]),
#        'not hulp and halp': set([5, 6]),
 #       'not (help and halp)': set([1, 2, 3, 4, 6, 7, 8]),
  #      '"help me please"': set([2]),
   #     '"help me please" or hulp': set([2, 3]),
    #    '"help me please" or (hulp and halp)': set([2]),
#        'help*': set([1, 2, 4, 5, 8]),
 #       'help or hulp*': set([1, 2, 3, 4, 5]),
  #      'help* and hulp': set([2]),
   #     'help and hulp* or hilp': set([2, 3, 4]),
    #    'help* or hulp or hilp or halp': set([1, 2, 3, 4, 5, 6, 8]),
     #   '(help or hulp*) and (hilp* or halp)': set([3, 4, 5]),
#        'help* and (hilp* or halp*)': set([4, 5]),
 #       '(help and (hilp* or halp)) or hulp*': set([2, 3, 4, 5]),
  #      'not help* and halp': set([6]),
   #     'not (help* and helpe*)': set([1, 2, 3, 4, 5, 6, 7]),
    #    '"help* me please"': set([2]),
     #   '"help* me* please" or hulp*': set([2, 3]),
      #  '"help me please*" or (hulp and halp)': set([2]),
#        '"help me please" not (hulp and halp)': set([2]),
 #       '"help me please" hulp': set([2]),
  #      'help and hilp and not holp': set([4]),
   #     'help hilp not holp': set([4]),
    #    'help hilp and not holp': set([4]),
    }

    docs = {
        1: 'help',
        2: 'help me please hulp',
        3: 'hulp hilp',
        4: 'help hilp',
        5: 'halp thinks he needs help',
        6: 'he needs halp',
        7: 'nothing',
        8: 'helper',
    }
        
    index = {
        'help': set((1, 2, 4, 5)),
        'me': set((2,)),
        'please': set((2,)),
        'hulp': set((2, 3,)),
        'hilp': set((3, 4,)),
        'halp': set((5, 6,)),
        'thinks': set((5,)),
        'he': set((5, 6,)),
        'needs': set((5, 6,)),
        'nothing': set((7,)),
        'helper': set((8,)),
    }

    def GetWord(self, word):
        if (self.index.has_key(word)):
            return self.index[word]
        else:
            return set()

    def GetWordWildcard(self, word):
        result = set()
        for item in self.index.keys():
            if word == item[0:len(word)]:
                result = result.union(self.index[item])
        return result

    def GetQuotes(self, search_string, tmp_result):
        result = set()
        for item in tmp_result:
            if self.docs[item].count(search_string):
                result.add(item)
        return result
    
    def GetNot(self, not_set):
        all = set(self.docs.keys())
        return all.difference(not_set)

    def Test(self):
        all_ok = True
        for item in self.tests.keys():
            print item
            r = self.Parse(item)
            e = self.tests[item]
            print 'Result: %s' % r
            print 'Expect: %s' % e
            if e == r:
                print 'Test OK'
            else:
                all_ok = False
                print '>>>>>>>>>>>>>>>>>>>>>>Test ERROR<<<<<<<<<<<<<<<<<<<<<'
            print ''
        return all_ok
            
if __name__=='__main__':
    if ParserTest().Test():
        print 'All tests OK'
    else:
        print 'One or more tests FAILED'
'''