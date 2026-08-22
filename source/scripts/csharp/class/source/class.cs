public class Counter
{
	public int count; // attribute

	public Counter(int start) // constructor
	{
		this.count = start;
	}

	public int Add(int value) // instance method
	{
		this.count += value;
		return this.count;
	}

	public static int Twice(int value) // static method
	{
		return value * 2;
	}
}
