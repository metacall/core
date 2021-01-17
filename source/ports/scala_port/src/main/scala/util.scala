package metacall

import com.sun.jna._

object util {
  private[metacall] class SizeT(value: Long)
      extends IntegerType(Native.SIZE_T_SIZE, value) {
    def this() = this(0)
  }
  private[metacall] object SizeT {
    def apply(value: Long) = new SizeT(value)
  }

  sealed class MetaCallException(message: String, val cause: Option[String])
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

  protected[metacall] def isNull(ptr: Pointer): Boolean =
    ptr == null || ptr == Pointer.NULL
}
