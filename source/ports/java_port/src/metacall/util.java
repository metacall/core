package metacall;

import com.sun.jna.*;

class util
{

	public static class SIZET extends IntegerType
	{
		private static final long serialVersionUID = 1L;

		public SIZET()
	    {
	      this(0);
	    }

	    public SIZET(long value)
	    {
	      super(Native.SIZE_T_SIZE, value, true);
	    }
	}

	public static interface FunctionPointer extends Callback
	{
		Pointer callback(SIZET argc, Pointer args, Pointer data);
	}

	public static interface ResolveCallback extends Callback
	{
		Pointer invoke(Pointer result, Pointer data);
	}
	public static interface RejectCallback extends Callback
	{
		Pointer invoke(Pointer error, Pointer data);
	}
}
