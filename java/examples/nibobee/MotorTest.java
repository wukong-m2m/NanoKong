import nanovm.nibobee.drivers.*;

/** MotorTest.java
  * 
  * test nibobee motor
  *
  * @author Torsten Röhl <informatics4kids.de>
  */

class MotorTest {

	          
	public static void main(String[] args) {

                        Clock.delayMilliseconds(1000);
                        Motor.setPWM( 700,700);
                        Clock.delayMilliseconds(1000);
                        Motor.setPWM( -700,-700);
                        Clock.delayMilliseconds(1000);
			Motor.stop();

		
	}
}
