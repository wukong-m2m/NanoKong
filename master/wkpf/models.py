# vim: ts=4 sw=4
import sqlite3

conn = None
def global_conn():
    global conn
    if not conn:
      print 'bootstraping database'
      conn = bootstrap_database()
      conn.row_factory = sqlite3.Row
    return conn

# in wuclasses, there are some with node_id NULL, that are wuclasses from XML
# also the same with properties, node_id might be NULL
def bootstrap_database():
    conn = sqlite3.connect(":memory:")
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS nodes
        (id INTEGER, 
         location TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuclasses
        (id INTEGER,
         name TEXT,
         node_id INTEGER, 
         virtual BOOLEAN,
         type TEXT,
         FOREIGN KEY(node_id) REFERENCES nodes(id))''')
    c.execute('''CREATE TABLE IF NOT EXISTS properties
        (id INTEGER, 
         name TEXT,
         datatype TEXT,
         access TEXT,
         value TEXT,
         wuclass_id INTEGER,
         node_id INTEGER,
         FOREIGN KEY(node_id) REFERENCES wuclasses(node_id),
         FOREIGN KEY(wuclass_id) REFERENCES wuclasses(id))''')
    c.execute('''CREATE TABLE IF NOT EXISTS wuobjects
        (id INTEGER PRIMARY KEY, 
         node_id INTEGER, 
         port_number INTEGER,
         wuclass_id INTEGER,
         FOREIGN KEY(wuclass_id) REFERENCES wuclasses(id)
         FOREIGN KEY(node_id) REFERENCES nodes(id))''')
    c.execute('''CREATE TABLE IF NOT EXISTS components
        (component_index INTEGER,
         location TEXT,
         group_size INTEGER,
         reaction_time REAL,
         type TEXT,
         application_hashed_name TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS links
        (id INTEGER PRIMARY KEY,
         from_component_index INTEGER,
         from_property_id INTEGER,
         to_component_index INTEGER,
         to_property_id INTEGER,
         to_wuclass_id INTEGER,
         FOREIGN KEY(to_wuclass_id) REFERENCES wuclasses(id),
         FOREIGN KEY(from_component_index) REFERENCES components(component_index),
         FOREIGN KEY(to_component_index) REFERENCES components(component_index))''')
    c.execute('''CREATE TABLE IF NOT EXISTS datatypes
        (id INTEGER PRIMARY KEY,
         name TEXT,
         type TEXT)''')
    c.execute('''CREATE TABLE IF NOT EXISTS datatype_values
        (id INTEGER PRIMARY KEY,
         value TEXT,
         datatype_id INTEGER,
         FOREIGN KEY(datatype_id) REFERENCES datatypes(id))''')
    conn.commit()
    return conn

class WuComponent:
    def __init__(self, component_index, location, group_size, reaction_time,
            type, application_hashed_name):
        self.index = component_index
        self.location = location
        self.group_size = group_size # int
        self.reaction_time = reaction_time # float
        self.type = type # wuclass name
        self.application_hashed_name = application_hashed_name
        self.instances = [] # WuObjects allocated on various Nodes after mapping
        self.heartbeatgroups = []

    def __repr__(self):
        return 'WuComponent#(component_index=%d,locationquery=%s,group_size=%d,reaction_time=%f,type=%s)' % (self.index, self.location, self.group_size, self.reaction_time, self.type)

class WuLink:
    def __init__(self, from_component_index, from_property_id,
            to_component_index, to_property_id, to_wuclass_id, id=None):
        self.id = id
        self.from_component_index = from_component_index
        self.from_property_id = from_property_id
        self.to_component_index = to_component_index
        self.to_property_id = to_property_id
        self.to_wuclass_id = to_wuclass_id

class WuType:
    @classmethod
    def all(cls):
        return WuType.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s=%s" % (x[0], x[1]),
                criteria.items()))
        c = global_conn().cursor()
        types = []
        if criteria:
            types_rows = c.execute("SELECT * from datatypes WHERE %s" % (criteria)).fetchall()
        else:
            types_rows = c.execute("SELECT * from datatypes").fetchall()
        for types_row in types_rows:
            values = []
            t = (types_row['id'],)
            values_rows = c.execute("SELECT value from datatype_values WHERE datatype_id=?", t).fetchall()
            values = [x['value'] for x in values_rows]

            types.append(WuType(types_row['name'], types_row['type'], values, types_row['id']))
            
        return types

    @classmethod
    def find(cls, id):
        return WuType.where(id=id)

    def __init__(self, name, type, values, id=None):
        self.id = id
        self.name = name
        self.type = type
        self.values = values

    def __repr__(self):
        return "WuType#(name=%s,type=%s,values=%s)" % (self.name, self.type,
                self.values)

    def save(self):
        c = global_conn().cursor()
        t = (self.id, self.name, self.type)
        c.execute("insert or replace into datatypes values (?,?,?)", t)
        global_conn().commit()
        self.id = c.lastrowid
        for value in self.values:
            t = (None, value, self.id)
            c.execute("insert or replace into datatype_values values (?,?,?)", t)
            global_conn().commit()

class Node:
    @classmethod
    def all(cls):
        return Node.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s=%s" % (x[0], x[1]),
                criteria.items()))
        c = global_conn().cursor()
        nodes = []
        if criteria:
            rows = c.execute("SELECT * from nodes WHERE %s" % (criteria)).fetchall()
        else:
            rows = c.execute("SELECT * from nodes").fetchall()
        for row in rows:
            wuclasses = []
            wuobjects = []

            r = (row['id'],)
            wuclasses_rows = c.execute("SELECT * from wuclasses WHERE node_id=?",
                    r).fetchall()
            for wuclasses_row in wuclasses_rows:

                properties = []
                p = (wuclasses_row['id'],)
                properties_rows = c.execute("SELECT * from properties WHERE wuclass_id=?",
                        p).fetchall()
                for properties_row in properties_rows:
                    properties.append(WuProperty(properties_row['id'],
                                properties_row['name'],
                                properties_row['datatype'],
                                properties_row['access'],
                                properties_row['value']))

                wuclasses.append(WuClass(wuclasses_row['id'],
                            wuclasses_row['name'],
                            bool(wuclasses_row['virtual']),
                            wuclasses_row['type'],
                            properties,
                            wuclasses_row['node_id']))

            t = (row['id'],)
            wuobjects_rows = c.execute("SELECT * from wuobjects WHERE node_id=?",
                    t).fetchall()
            for wuobjects_row in wuobjects_rows:
                wuclass = filter(lambda x: x.id == wuobjects_row['wuclass_id'],
                        wuclasses)[0]
                wuobjects.append(WuObject(wuobjects_row['node_id'], 
                            wuobjects_row['port_number'],
                            wuclass,
                            wuobjects_row['id']))

            nodes.append(Node(row['id'], row['location'], 
                        wuclasses, wuobjects))
            
        return nodes

    @classmethod
    def find(cls, id):
        return Node.where(id=id)

    def __init__(self, id, location, wuclasses=[], wuobjects=[]):
        self.id = id
        self.location = location
        self.wuclasses = wuclasses
        self.wuobjects = wuobjects

    def __repr__(self):
        return 'Node#(id="%d",location="%s",wuclasses=%s,wuobjects=%s)' % (self.id, self.location, self.wuclasses, self.wuobjects)

    def __eq__(self, other):
        if isinstance(other, Node):
            return other.id == self.id and other.location == self.location and other.wuclasses == self.wuclasses and other.wuobjects == self.wuobjects
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def isResponding(self):
        return self.wuclasses or self.wuobjects

    def save(self):
        c = global_conn().cursor()
        t = (self.id, self.location,)
        c.execute("INSERT OR REPLACE INTO nodes VALUES (?,?)", t)
        global_conn().commit()
        # try recursion for now
        map(lambda x: x.save(), self.wuclasses)
        map(lambda x: x.save(), self.wuobjects)
        #t = map(lambda x: (x.id, x.node_id, x.virtual, x.type,), self.wuclasses)
        #c.executemany("INSERT OR REPLACE INTO wuclasses VALUES (?,?,?,?)", t)
        #t = map(lambda x: (x.node_id, x.port_number,x.wuclass.id), self.wuobjects)
        #c.executemany("INSERT OR REPLACE INTO wuobjects VALUES (?,?,?)", t)

class WuClass:
    @classmethod
    def all(cls):
        return WuClass.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s=%s" % (x[0], x[1]), criteria.items()))
        c = global_conn().cursor()
        if criteria:
            wuclasses_rows = c.execute("SELECT * from wuclasses WHERE %s" % (criteria)).fetchall()
        else:
            wuclasses_rows = c.execute("SELECT * from wuclasses").fetchall()
        wuclasses = []
        for wuclasses_row in wuclasses_rows:

            properties = []
            p = (wuclasses_row['id'],)
            properties_rows = c.execute("SELECT * from properties WHERE wuclass_id=?",
                    p).fetchall()
            for properties_row in properties_rows:
                properties.append(WuProperty(properties_row['id'],
                            properties_row['name'],
                            properties_row['datatype'],
                            properties_row['access'],
                            properties_row['value']))

            wuclasses.append(WuClass(wuclasses_row['id'],
                        wuclasses_row['name'],
                        bool(wuclasses_row['virtual']),
                        wuclasses_row['type'],
                        properties,
                        wuclasses_row['node_id']))
        return wuclasses

    @classmethod
    def find(cls, id):
        return WuClass.where(id=id)

    def __init__(self, id, name, virtual, type, properties=[], node_id=None):
        self.id = id
        self.name = name
        self.node_id = node_id
        self.virtual = virtual
        self.type = type
        self.properties = properties
        def func(x):
          x.wuclass = self
        map(func, self.properties)

    def __repr__(self):
        return
        'WuClass#(id="%d",name="%s",node_id="%s",virtual=%r,type="%s",properties=%s)' % (self.id, self.name, self.node_id, self.virtual, self.type, self.properties)

    def __eq__(self, other):
        if isinstance(other, WuClass):
            return other.id == self.id and other.node_id == self.node_id and other.virtual == self.virtual and other.type == self.type and other.properties and self.properties
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def save(self):
        c = global_conn().cursor()
        t = (self.id, self.name, self.node_id, self.virtual, self.type,)
        c.execute("INSERT OR REPLACE INTO wuclasses VALUES (?,?,?,?,?)", t)
        map(lambda x: x.save(), self.properties)

class WuProperty:
    @classmethod
    def all(cls):
        return WuProperty.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s=%s" % (x[0], x[1]), criteria.items()))
        c = global_conn().cursor()
        if criteria:
            properties_rows = c.execute("SELECT * from properties WHERE %s" % (criteria)).fetchall()
        else:
            properties_rows = c.execute("SELECT * from properties").fetchall()
        properties = []
        for properties_row in properties_rows:
            properties.append(WuProperty(properties_row['wuclass_id'],
                        properties_row['name'],
                        properties_row['datatype'],
                        properties_row['access'],
                        properties_row['value']))
        return properties

    @classmethod
    def find(cls, id):
        return WuProperty.where(id=id)

    def __init__(self, id, name, datatype, access, value, status=None):
        self.id = id
        self.name = name
        self.datatype = datatype
        self.access = access
        self.value = value # default or current are the same variable
        self.status = status
        self.wuclass = None

    def __repr__(self):
        return 'WuProperty#(id="%d",name="%s",datatype=%s,access="%s",value=%s)' % (self.id, self.name, self.datatype, self.access, self.value)

    def __eq__(self, other):
        if isinstance(other, WuProperty):
            return other.id == self.id and other.name == self.name and other.datatype == self.datatype and other.access == self.access and other.value and self.value
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def save(self):
        c = global_conn().cursor()
        t = (self.id, self.name, self.datatype, self.access, self.value,
                self.wuclass.id, self.wuclass.node_id,)
        c.execute("INSERT OR REPLACE INTO properties VALUES (?,?,?,?,?,?,?)", t)
        global_conn().commit()

class WuObject:
    @classmethod
    def all(cls):
        return WuObject.where()

    @classmethod
    def where(cls, **criteria):
        criteria = " AND ".join(map(lambda x: "%s=%s" % (x[0], x[1]), criteria.items()))
        c = global_conn().cursor()
        if criteria:
            wuobjects_rows = c.execute("SELECT * from wuobjects WHERE %s" % (criteria)).fetchall()
        else:
            wuobjects_rows = c.execute("SELECT * from wuobjects").fetchall()

        wuobjects = []
        for wuobjects_row in wuobjects_rows:
            wuclasses = WuClass.all()
            wuclass = filter(lambda x: x.id == wuobjects_row['wuclass_id'],
                    wuclasses)[0]
            wuobjects.append(WuObject(wuobjects_row['node_id'], 
                        wuobjects_row['port_number'],
                        wuclass,
                        wuobjects_row['id']))
        return wuobjects

    @classmethod
    def find(cls, id):
        return WuObject.where(id=id)

    def __init__(self, node_id, port_number, wuclass, id=None):
        self.id = id
        self.node_id = node_id
        self.port_number = port_number
        self.wuclass = wuclass

    def __repr__(self):
        return 'WuObject#(id="%d",node_id="%s",port_number=%d,wuclass="%s")' % (self.id, self.node_id, self.port_number, self.wuclass)

    def __eq__(self, other):
        if isinstance(other, WuObject):
            return other.id == self.id and other.node_id == self.node_id and other.port_number == self.port_number and other.wuclass == self.wuclass
        return NotImplemented

    def __ne__(self, other):
        result = self.__eq__(other)
        if result is NotImplemented:
            return result
        return not result

    def save(self):
        c = global_conn().cursor()
        t = (self.id, self.node_id, self.port_number, self.wuclass.id,)
        c.execute("INSERT OR REPLACE INTO wuobjects VALUES (?,?,?,?)", t)
        global_conn().commit()
