import sys, os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import unittest
from wkpf.wkapplication import constructHeartbeatGroups

class TestConstructHeartbeatGroups(unittest.TestCase):
  def setUp(self):
    pass

  def test_empty(self):
    heartbeatGroups = []
    routingTable = {}
    candidateSet = []
    constructHeartbeatGroups(heartbeatGroups, routingTable, candidateSet)
    self.assertEqual(heartbeatGroups, [[]])

  def test_two(self):
    heartbeatGroups = []
    routingTable = {2: [3], 3: [2]}
    candidateSet = [2, 3]
    constructHeartbeatGroups(heartbeatGroups, routingTable, candidateSet)
    self.assertEqual(heartbeatGroups, [[2, 3]])

  def test_three(self):
    heartbeatGroups = []
    routingTable = {2: [3, 4], 3: [2, 4], 4:[3, 2]}
    candidateSet = [2, 3, 4]
    constructHeartbeatGroups(heartbeatGroups, routingTable, candidateSet)
    self.assertEqual(heartbeatGroups, [[2, 3, 4]])

  def test_three_but_two_first(self):
    heartbeatGroups = []
    routingTable = {2: [3, 4], 3: [2], 4:[2]}
    candidateSet = [2, 3, 4]
    constructHeartbeatGroups(heartbeatGroups, routingTable, candidateSet)
    self.assertEqual(heartbeatGroups, [[2, 3, 4]])

  def test_three_but_three_first(self):
    heartbeatGroups = []
    routingTable = {2: [3, 4], 3: [2], 4:[2]}
    candidateSet = [3, 4, 2]
    constructHeartbeatGroups(heartbeatGroups, routingTable, candidateSet)
    self.assertEqual(heartbeatGroups, [[3, 2], [4]])

if __name__ == '__main__':
  unittest.main()
