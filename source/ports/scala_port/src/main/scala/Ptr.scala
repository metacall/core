package metacall

import metacall.util._
import com.sun.jna._
import cats._, cats.effect._

/** Type class for creating pointers to MetaCall values */
trait Create[A, P <: Ptr[A]] {
  def create[F[_]](value: A)(implicit FE: ApplicativeError[F, Throwable]): F[P]
}

trait Get[A, P <: Ptr[A]] {
  def get[F[_]](ptr: P)(implicit FE: ApplicativeError[F, Throwable]): F[A]
}

/** Represents a native pointer */
sealed trait Ptr[A] {
  val ptr: Pointer
  val ptrType: PtrType
}
object Ptr {

  /** Create a managed pointer to a MetaCall value */
  def from[A, P <: Ptr[A], F[_]](value: A)(implicit
      FE: ApplicativeError[F, Throwable],
      C: Create[A, P]
  ): Resource[F, P] =
    Resource.make(C.create[F](value)) { v =>
      try FE.pure(Bindings.instance.metacall_value_destroy(v.ptr))
      catch {
        case e: Throwable =>
          FE.raiseError(new DestructionError(v.ptr, Some(e.getMessage())))
      }
    }

}

/** Represents pointer types
  * See https://github.com/metacall/core/blob/develop/source/metacall/include/metacall/metacall_value.h#L44
  * Unimplemented MetaCall types:
  * ```c
  * enum metacall_value_id {
  *   ...
  *   METACALL_PTR		= 11,
  *   METACALL_FUTURE	 = 12,
  *   METACALL_FUNCTION	= 13,
  *   ...
  *   METACALL_CLASS	  = 15,
  *   METACALL_OBJECT	  = 16,
  *   ...
  * };
  * ```
  */
sealed trait PtrType {
  val id: Int
}

private[metacall] final class BoolPtr(val ptr: Pointer) extends Ptr[Boolean] {
  val ptrType = BoolPtrType
}
object BoolPtrType extends PtrType {
  val id = 0
}

private[metacall] final class CharPtr(val ptr: Pointer) extends Ptr[Char] {
  val ptrType = CharPtrType
}
object CharPtrType extends PtrType {
  val id = 1
}

private[metacall] final class ShortPtr(val ptr: Pointer) extends Ptr[Short] {
  val ptrType = ShortPtrType
}
object ShortPtrType extends PtrType {
  val id = 2
}

private[metacall] final class IntPtr(val ptr: Pointer) extends Ptr[Int] {
  val ptrType = IntPtrType
}
object IntPtrType extends PtrType {
  val id = 3
}

private[metacall] final class LongPtr(val ptr: Pointer) extends Ptr[Long] {
  val ptrType = LongPtrType
}
object LongPtrType extends PtrType {
  val id = 4
}

private[metacall] final class FloatPtr(val ptr: Pointer) extends Ptr[Float] {
  val ptrType = FloatPtrType
}
object FloatPtrType extends PtrType {
  val id = 5
}

private[metacall] final class DoublePtr(val ptr: Pointer) extends Ptr[Double] {
  val ptrType = DoublePtrType
}
object DoublePtrType extends PtrType {
  val id = 6
}

private[metacall] final class StringPtr(val ptr: Pointer) extends Ptr[String] {
  val ptrType = StringPtrType
}
object StringPtrType extends PtrType {
  val id = 7
}

private[metacall] final class ArrayPtr(val ptr: Pointer) extends Ptr[Array[Pointer]] {
  val ptrType = ArrayPtrType
}
object ArrayPtrType extends PtrType {
  val id = 9
}

private[metacall] final class MapPtr(val ptr: Pointer) extends Ptr[Array[Pointer]] {
  val ptrType = MapPtrType
}
object MapPtrType extends PtrType {
  val id = 10
}

private[metacall] final class NullPtr(val ptr: Pointer) extends Ptr[Null] {
  val ptrType = NullPtrType
}
object NullPtrType extends PtrType {
  val id = 14
}

private[metacall] final class SizePtr(val ptr: Pointer) extends Ptr[SizeT] {
  val ptrType = SizePtrType
}
object SizePtrType extends PtrType {
  val id = 17
}

object InvalidPtrType extends PtrType {
  val id = 18
}
