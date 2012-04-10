public class BubbleSort
{
	public static void main(String args[])
	{
		int i,j,l;

		int NUMNUMBERS = 256;
		int numbers[] = new int[NUMNUMBERS];

    System.out.println("START");

		for (l=0; l<1; l++)
		{

			for (i=0; i<NUMNUMBERS; i++)
				numbers[i] = (int)(NUMNUMBERS - 1 - i);

			for (i=0; i<NUMNUMBERS; i++)
			{
				for (j=0; j<NUMNUMBERS-i-1; j++)
					if (numbers[j]>numbers[j+1])
					{
						int temp = numbers[j];
						numbers[j] = numbers[j+1];
						numbers[j+1] = temp;
					}
			}

		}
		
    System.out.println("END");
	}
}
