package metacall;

import com.sun.jna.*;

class Utils
{

	public static class SizeT extends IntegerType
	{
		private static final long serialVersionUID = 1L;

		public SizeT()
	    {
	      this(0);
	    }

	    public SizeT(long value)
	    {
	      super(Native.SIZE_T_SIZE, value, true);
	    }
	}

	public static interface FunctionPointer extends Callback
	{
		Pointer callback(SizeT argc, Pointer args, Pointer data);
	}

	public static interface ResolveCallback extends Callback
	{
		Pointer invoke(Pointer result, Pointer data);
	}
	public static interface RejectCallback extends Callback
	{
		Pointer invoke(Pointer error, Pointer data);
	}

	// TODO
	//MetacallException
	//AllocationError
	//DestrucitonError

	boolean isNull(Pointer ptr)
	{
		return (ptr == null || ptr == Pointer.NULL)? true:false;
	}

}
