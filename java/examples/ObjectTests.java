import java.io.*;

class ObjectTests {
	private static ObjectTests obj1;
	private static ObjectTests obj2;
	
	String value;
	
	public ObjectTests(String v) {
		this.value = v;
	}
	
	public void print() {
		System.out.println(this.value);
	}
	
	public static void main(String[] args) throws IOException {
		obj1 = new ObjectTests("een");
		obj2 = new ObjectTests("twee");
		
		System.out.println("TestInvokeMethod, expecting: een, een, twee");
		TestInvokeMethod(obj1);
		System.out.println("TestReferenceEquality, expecting: 1");
		TestReferenceEquality(obj1);
	}
	
	public static void TestInvokeMethod(ObjectTests obj) {
		obj.print();
		obj1.print();
		obj2.print();
	}
	
	public static void TestReferenceEquality(ObjectTests obj) {
		if (obj == obj1)
			System.out.println("1");
		if (obj == obj2)
			System.out.println("2");
	}
}
