package metacall;

import com.sun.jna.*;

// TODO
// case class Script(runtime: Runtime, filePath: String)

Class Util
{
    //size_t definition
    Class SizeT extends IntegerType
    {
        public SIZET()
        {
          this(0);
        }

        public SIZET(long value)
        {
          super(Native.SIZE_T_SIZE, value, true);
        }
    }

    Class FunctionPointer extends Callback
    {
      return Pointer callback(SizeT argc, Pointer args, Pointer data);
    }

    // TODO
    /*  sealed class MetaCallException(message: String, val cause: Option[String])
          extends Exception(message + cause.map(" Cause: " + _).getOrElse(""))
      class AllocationError[A](value: Option[A], cause: Option[String])
          extends MetaCallException(
            s"Allocation Error${value.map(v => ": Failed to allocate" + v.toString()).getOrElse("")}",
            cause
          )
      class DestructionError(ptr: Pointer, cause: Option[String])
          extends MetaCallException(
            s"Destruction Error: Destroying value at pointer $ptr. ${cause.getOrElse("")}",
            cause
          )
    */


    boolean isNull(Pointer ptr) = (ptr == null || ptr == Pointer.NULL)?;


}
