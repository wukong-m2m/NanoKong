/*
  inputDemo.java
 */

import nanovm.avr.*;

class javainterrupt {
  public static void main(String[] args) {

		AVR.portD.setInput(0);	//INT0,pull high
		AVR.portD.setInput(1);	//INT1,pull high
		AVR.portD.setInput(2);	//INT2,pull high
		AVR.portD.setInput(3);	//INT3,pull high
		AVR.portB.setInput(0);	//PCINT0,pull high
		AVR.setIctrlINT(3,2);	//set INT3 to mode2 (falling edge generate interrupt, initial mode is GND generate interrupt)

		while(true)
		{
			//touch INT0 (digital 21) to GND create interrupt
			//touch PCINT0 (digital 53) to GND/VCC create interrupt
			if(AVR.getIflagINT(0)==1)//INT0
			{
				AVR.clrIflagINT(0);//clear interrupt flag
				System.out.println("interrupt INT0");
			}
			else if(AVR.getIflagINT(1)==1)//INT1
			{
				AVR.clrIflagINT(1);//clear interrupt flag
				System.out.println("interrupt INT1");
			}
			else if(AVR.getIflagINT(2)==1)//INT2
			{
				AVR.clrIflagINT(2);//clear interrupt flag
				System.out.println("interrupt INT2");
			}
			else if(AVR.getIflagINT(3)==1)//INT3
			{
				AVR.clrIflagINT(3);//clear interrupt flag
				System.out.println("interrupt INT3");
			}
			else if(AVR.getIflagPCINTA(0)==1)//PCINT0
			{
				AVR.clrIflagPCINTA(0);//clear interrupt flag
				System.out.println("interrupt PCINT0");
			}
			System.out.println("waiting");
			Timer.wait(1000);
		}

  }
}


