import nanovm.nibobee.drivers.*;

/** Calibration.java
  * 
  * 
  *
  * @author Torsten Röhl <informatics4kids.de>
  */

class Calibration {

	public static void main(String[] args) {

		while (true) {

			int value = Sensor.getLeft();
			if(value != 0){			
				LineDetector.calibrateWhite();
				Leds.setStatus(Leds.LEFT_RED, true);
				Leds.setStatus(Leds.LEFT_YELLOW, true);
                                Clock.delayMilliseconds(500);				
			}
			
			value = Sensor.getRight();
			if(value != 0){
				LineDetector.calibrateBlack();
				Leds.setStatus(Leds.RIGHT_RED, true);
				Leds.setStatus(Leds.RIGHT_YELLOW, true);
                                Clock.delayMilliseconds(500);				
			}
			
		}

	}
}
