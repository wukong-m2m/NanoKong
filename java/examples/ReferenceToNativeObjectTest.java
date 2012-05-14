/*
  inputDemo.java
 */

import nanovm.avr.*;

class ReferenceToNativeObjectTest {
  public static void main(String[] args) {
		System.out.println("System clock = " + AVR.getClock() + "000 Hz");
		System.out.println("Blinking LED");

		Timer.setPrescaler(Timer.DIV8);  // timer base = 1/8 system clock = 16M/8=2M
		Timer.setSpeed(2000);    // timer = 1/2000 base -> 2M/2000=1000 Hz

		AVR.portA.setInput(0);
		AVR.portB.setOutput(0);
		
		while(true)
		{
			int in_test;

			//you can connect b to a, then see the printing message in monitor
			AVR.portB.setBit(0);
			Timer.wait(1000);
			in_test=AVR.portA.getInput(0);//get input data from PA0
			System.out.println(in_test);
			AVR.portB.clrBit(0);
			Timer.wait(1000);
			in_test=AVR.portA.getInput(0);//get input data from PA0
			System.out.println(in_test);
		}

  }
}


