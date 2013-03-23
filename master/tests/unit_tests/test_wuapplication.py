import os, sys
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '../..'))
sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import unittest
import test_environment
from wkpf.wuapplication import *

xml = '''
<application name="123">
<page title="_first_">
    <component type="Threshold" instanceId="19" x="343" y="206" w="100" h="100">
        <link fromProperty="output" toInstanceId="22" toProperty="on_off"/>
        <location requirement="WuKong" />
        <group_size requirement="1" />
        <reaction_time requirement="2" />
        <actionProperty  />
        <signalProperty  />
    </component>
    <component type="Numeric_Controller" instanceId="20" x="80" y="237" w="100" h="100">
        <link fromProperty="output" toInstanceId="19" toProperty="threshold"/>
        <location requirement="WuKong" />
        <group_size requirement="1" />
        <reaction_time requirement="2" />
        <actionProperty output="gg"  />
        <signalProperty output="gg"  />
    </component>
    <component type="Light_Sensor" instanceId="21" x="76" y="397" w="100" h="100">
        <link fromProperty="current_value" toInstanceId="19" toProperty="value"/>
        <location requirement="WuKong" />
        <group_size requirement="2" />
        <reaction_time requirement="2" />
        <actionProperty refresh_rate="56"  />
        <signalProperty current_value="11" refresh_rate="323"  />
    </component>
    <component type="Light_Actuator" instanceId="22" x="633" y="224" w="100" h="100">
        <location requirement="WuKong" />
        <group_size requirement="2" />
        <reaction_time requirement="2" />
        <actionProperty on_off="true"  />
        <signalProperty on_off="false"  />
    </component>
</page>
</application>
'''

class TestWuApplication(unittest.TestCase):
  def test_parseApplication(self):
    application = WuApplication('455', '123')
    application.setFlowDom(parseString(xml))
    application.parseApplication()

    self.assertEqual(application.name, '123')
    self.assertEqual(len(application.changesets.components), 4)
    self.assertTrue((u'output', u'gg') in application.changesets.components[1].properties_with_default_values)
    self.assertTrue((u'refresh_rate', u'323') in application.changesets.components[2].properties_with_default_values)
    self.assertTrue((u'on_off', u'false') in application.changesets.components[3].properties_with_default_values)

if __name__ == '__main__':
  unittest.main()
