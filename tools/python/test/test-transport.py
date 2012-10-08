import sys, os
import unittest
sys.path.append(os.path.abspath("../"))
from transport import *

class TestTransportFunctions(unittest.TestCase):
    def setUp(self):
        self.zwaveAgent = ZwaveAgent.init()
        self.agent = Agent.init()

    def test_init(self):
        self.agent.add(self.zwaveAgent)
        assertIsInstance(self.zwaveAgent.dispatcher(), Agent)

    def test_send(self):
        reply = self.zwaveAgent.send(58, pynvc.WKPF_GET_WUCLASS_LIST, [], [pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R])
        self.assertEqual(reply.command, pynvc.WKPF_GET_WUCLASS_LIST_R)

    def test_deferSend(self):
        defer = self.zwaveAgent.deferSend(58, pynvc.WKPF_GET_WUCLASS_LIST, [], [pynvc.WKPF_GET_WUCLASS_LIST_R, pynvc.WKPF_ERROR_R], callback=self.test_callback)

    def test_callback(self, message):
        self.assertEqual(message.command, pynvc.WKPF_GET_WUCLASS_LIST_R)


if __name__ == '__main__':
    unittest.main()
