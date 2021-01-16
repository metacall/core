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
  */
sealed trait PtrType {
  val id: Int
}

private[metacall] final class IntPtr(val ptr: Pointer) extends Ptr[Int]
object IntPtr extends PtrType {
  val id = 3
}

private[metacall] final class StringPtr(val ptr: Pointer) extends Ptr[String]
object StringPtr extends PtrType {
  val id = 7
}
