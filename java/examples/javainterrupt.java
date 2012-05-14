/*
  inputDemo.java
 */

import nanovm.avr.*;

class javainterrupt {
  public static void main(String[] args) {

/*
AVR.setPinIOMode( AVR.ANALOG8 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG9 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG10 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG11 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG12 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG13 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG14 , AVR.INPUT );
AVR.setPinIOMode( AVR.ANALOG15 , AVR.INPUT );
AVR.setPinInterruptMode( AVR.ANALOG8 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG9 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG10 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG11 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG12 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG13 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG14 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.ANALOG15 , AVR.ANYCHANGE );


AVR.setPinIOMode( AVR.DIGITAL50 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL51 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL10 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL11 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL12 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL13 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL52 , AVR.INPUT );
AVR.setPinIOMode( AVR.DIGITAL53 , AVR.INPUT );
AVR.setPinInterruptMode( AVR.DIGITAL50 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL51 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL10 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL11 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL12 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL13 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL52 , AVR.ANYCHANGE );
AVR.setPinInterruptMode( AVR.DIGITAL53 , AVR.ANYCHANGE );
*/
int x1=1;
int temp=5;
x1 &= AVR.setPinIOMode( AVR.DIGITAL21 , AVR.INPUT );
x1 &= AVR.setPinIOMode( AVR.DIGITAL20 , AVR.INPUT );
x1 &= AVR.setPinIOMode( AVR.DIGITAL19 , AVR.INPUT );
x1 &= AVR.setPinIOMode( AVR.DIGITAL18 , AVR.INPUT );
x1 &= AVR.setPinIOMode( AVR.DIGITAL2 , AVR.INPUT );
x1 &= AVR.setPinIOMode( AVR.DIGITAL3 , AVR.INPUT );

x1 &= AVR.setPinInterruptMode( AVR.DIGITAL21 , AVR.GND );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL20 , AVR.GND );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL19 , AVR.GND );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL18 , AVR.GND );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL2 , AVR.GND );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL3 , AVR.GND );

x1 &= AVR.setPinIOMode( AVR.ANALOG15 , AVR.INPUT );
x1 &= AVR.setPinInterruptMode( AVR.ANALOG15 , AVR.ANYCHANGE );
x1 &= AVR.setPinIOMode( AVR.DIGITAL14 , AVR.INPUT );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL14 , AVR.ANYCHANGE );
x1 &= AVR.setPinIOMode( AVR.DIGITAL15 , AVR.INPUT );
x1 &= AVR.setPinInterruptMode( AVR.DIGITAL15 , AVR.ANYCHANGE );

	if( x1==1 ) {

		while(true)
		{
AVR.getClock();
			int mask_INT;
			mask_INT=AVR.select(AVR.EVENT_INT,1000);
//			mask_INT=AVR.select(AVR.EVENT_PCINTA,1000);
//			mask_INT=AVR.select(AVR.EVENT_PCINTC,1000);
			System.out.println(mask_INT);
		
			if( (mask_INT & AVR.MASK_INT0)!=0 )
			{System.out.println("INT0");}
			if( (mask_INT & AVR.MASK_INT1)!=0 )
			{System.out.println("INT1");}
			if( (mask_INT & AVR.MASK_INT2)!=0 )
			{System.out.println("INT2");}
			if( (mask_INT & AVR.MASK_INT3)!=0 )
			{System.out.println("INT3");}
			if( (mask_INT & AVR.MASK_INT4)!=0 )
			{System.out.println("INT4");}
			if( (mask_INT & AVR.MASK_INT5)!=0 )
			{System.out.println("INT5");}
			//if( (mask_INT & AVR.MASK_PCINT16)!=0 )
			//{System.out.println("PCINT16");}

			System.out.println("sleep after"+temp);
			//Timer.wait(1000);
			temp--;
			if(temp==0)
			{
				System.out.println("go to sleep mode");
				System.out.println("make interrupt to wake up");
				temp=5;
				AVR.sleep(5000);
				//AVR.sleep(10);
			}
			//AVR.setPinInterruptMode( AVR.DIGITAL20 , AVR.DISABLE );
		}
	}
  }
}


