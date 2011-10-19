/*
  inputDemo.java
 */

import nanovm.avr.*;

class TimerDemo {
  public static void main(String[] args) {
		System.out.println("System clock = " + AVR.getClock() + "000 Hz");
		System.out.println("Blinking LED");

		System.out.println("default Timer.wait unit = 1ms");
	
		while(true)
		{
			Timer.setSpeed(2000);    // timer = 1/2000 base -> 2M/2000=1000 Hz ->T=1ms
			System.out.println("wait 1 second");
			Timer.wait(1000);

			Timer.setSpeed(4000);    // timer = 1/4000 base -> 2M/4000=500 Hz ->T=2ms
			System.out.println("wait 1 second");
			Timer.wait(500);


			Timer.setSpeed(4000);    // timer = 1/4000 base -> 2M/4000=500 Hz ->T=2ms
			System.out.println("wait 2 second");
			Timer.wait(1000);
		}

  }
}


