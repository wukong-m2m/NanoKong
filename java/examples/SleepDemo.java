/*
  inputDemo.java
 */

import nanovm.avr.*;

class SleepDemo {
  public static void main(String[] args) {

	int x1=1;
	//enable INT0.1, we can wake up from INT0.1
	x1 &= AVR.setPinIOMode( AVR.DIGITAL2 , AVR.INPUT );
	x1 &= AVR.setPinIOMode( AVR.DIGITAL0 , AVR.OUTPUT );
	x1 &= AVR.setPinInterruptMode( AVR.DIGITAL2 , AVR.GND );
	//x1 &= AVR.setPinInterruptMode( AVR.DIGITAL20 , AVR.GND );
AVR.digitalWrite( AVR.DIGITAL0 , 0);
	int time=10000;
	if(x1==1)//no error during set pin
	{
		while(true)
		{
			System.out.println("sleep " + time + "ms");
			//System.out.println("start sleep " + time + "ms");
			AVR.sleep(time);
			System.out.println("~~finish");
			Timer.wait(500);

		}
	}

  }
}


