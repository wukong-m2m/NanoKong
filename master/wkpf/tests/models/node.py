# vim: ts=4 sw=4
import os, sys
sys.path.append(os.path.abspath("../../../"))
import unittest
import wkpf.models
import sure

class TestNode(unittest.TestCase):
    def test_adding(self):
        wuclasses = []
        properties = [wkpf.models.WuProperty(3, "value", "short", "readwrite", "30"),
                   wkpf.models.WuProperty(4, "output", "boolean", "readonly", "false")]
        wuclasses.append(wkpf.models.WuClass(3, 4, True, "soft", properties))

        wuobjects = []
        wuobjects.append(wkpf.models.WuObject(5, 4, 2, wuclasses[0]))

        node = wkpf.models.Node(4, 'WuKong', wuclasses, wuobjects)

        node.save()

        self.assertTrue(len(wkpf.models.Node.all()) == 1)
        self.assertTrue([node] == wkpf.models.Node.all())

if __name__ == "__main__":
    unittest.main()
