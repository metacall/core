package metacall

import metacall.util._
import com.sun.jna._
import cats._, cats.implicits._, cats.effect._

/** Create a [[Ptr]] to MetaCall value of type [[A]] */
trait Create[A] {
  def create[F[_]](value: A)(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[A]]
}
object Create {
  def apply[A](implicit C: Create[A]) = C
}

/** Gets the value of a [[Ptr]]. */
trait Get[A] {

  /** Get the primitive representation of the pointer's value */
  def primitive[F[_]](ptr: Ptr[A])(implicit FE: ApplicativeError[F, Throwable]): F[A]

  /** Get the pointer's high-level [[metacall.Value]] representation */
  def value[F[_]](ptr: Ptr[A])(implicit FE: ApplicativeError[F, Throwable]): F[Value]

}
object Get {
  def apply[A](implicit G: Get[A]) = G
}

/** Represents a native pointer. */
sealed trait Ptr[A] {
  val ptr: Pointer
  val ptrType: PtrType
}
object Ptr {

  /** Create a managed pointer to a MetaCall value */
  def from[A, F[_]](value: A)(implicit
      FE: ApplicativeError[F, Throwable],
      FD: Defer[F],
      C: Create[A]
  ): Resource[F, Ptr[A]] =
    Resource.make(C.create[F](value)) { vPtr =>
      FD.defer {
        try FE.pure(Bindings.instance.metacall_value_destroy(vPtr.ptr))
        catch {
          case e: Throwable =>
            FE.raiseError(new DestructionError(vPtr.ptr, Some(e.getMessage())))
        }
      }
    }

  /** Create a managed pointer to an array containing the values */
  def fromVector[A, F[_]](vec: Vector[A])(implicit
      FE: ApplicativeError[F, Throwable],
      FD: Defer[F],
      CA: Create[A],
      CR: Create[Array[Pointer]]
  ): Resource[F, Ptr[Array[Pointer]]] = {
    val elemPtrs = vec.traverse(a => CA.create[F](a).map(_.ptr))
    Resource.suspend(elemPtrs.map(_.toArray).map(from[Array[Pointer], F]))
  }

  import metacall.instances._

  private[metacall] def fromValueUnsafe[F[_]](v: Value)(implicit
      FE: MonadError[F, Throwable]
  ): F[Ptr[_]] = v match {
    case CharValue(value)    => Create[Char].create[F](value).widen[Ptr[_]]
    case StringValue(value)  => Create[String].create[F](value).widen[Ptr[_]]
    case ShortValue(value)   => Create[Short].create[F](value).widen[Ptr[_]]
    case IntValue(value)     => Create[Int].create[F](value).widen[Ptr[_]]
    case LongValue(value)    => Create[Long].create[F](value).widen[Ptr[_]]
    case FloatValue(value)   => Create[Float].create[F](value).widen[Ptr[_]]
    case DoubleValue(value)  => Create[Double].create[F](value).widen[Ptr[_]]
    case SizeTValue(value)   => Create[SizeT].create[F](SizeT(value)).widen[Ptr[_]]
    case BooleanValue(value) => Create[Boolean].create[F](value).widen[Ptr[_]]
    case ArrayValue(value) => {
      val elemPtrs = value.traverse(fromValueUnsafe[F](_).map(_.ptr)).map(_.toArray)
      elemPtrs.flatMap(Create[Array[Pointer]].create[F](_)(FE).widen[Ptr[_]])
    }
    case MapValue(value) => {
      val tuplePtrs = value.toVector
        .traverse { case (k, v) =>
          (fromValueUnsafe[F](k) product fromValueUnsafe[F](v))
            .map { case (p1, p2) => p1.ptr -> p2.ptr }
        }
        .map(_.toArray)
      tuplePtrs.flatMap(Create[Array[(Pointer, Pointer)]].create[F]).widen[Ptr[_]]
    }
    case NullValue => Create[Null].create[F](null).widen[Ptr[_]]
  }

  def fromValue[F[_]](v: Value)(implicit
      FE: MonadError[F, Throwable],
      FD: Defer[F]
  ): Resource[F, Ptr[_]] = Resource.make(fromValueUnsafe[F](v)) { vPtr =>
    FD.defer {
      try FE.pure(Bindings.instance.metacall_value_destroy(vPtr.ptr))
      catch {
        case e: Throwable =>
          FE.raiseError(new DestructionError(vPtr.ptr, Some(e.getMessage())))
      }
    }
  }

  private[metacall] def fromPrimitive[F[_]](ptr: Pointer)(implicit
      FE: ApplicativeError[F, Throwable]
  ): F[Ptr[_]] = PtrType.of(ptr) match {
    case BoolPtrType   => new BoolPtr(ptr).pure[F].widen[Ptr[_]]
    case CharPtrType   => new CharPtr(ptr).pure[F].widen[Ptr[_]]
    case ShortPtrType  => new ShortPtr(ptr).pure[F].widen[Ptr[_]]
    case IntPtrType    => new IntPtr(ptr).pure[F].widen[Ptr[_]]
    case LongPtrType   => new LongPtr(ptr).pure[F].widen[Ptr[_]]
    case FloatPtrType  => new FloatPtr(ptr).pure[F].widen[Ptr[_]]
    case DoublePtrType => new DoublePtr(ptr).pure[F].widen[Ptr[_]]
    case StringPtrType => new StringPtr(ptr).pure[F].widen[Ptr[_]]
    case ArrayPtrType  => new ArrayPtr(ptr).pure[F].widen[Ptr[_]]
    case MapPtrType    => new MapPtr(ptr).pure[F].widen[Ptr[_]]
    case NullPtrType   => new NullPtr(ptr).pure[F].widen[Ptr[_]]
    case SizePtrType   => new SizePtr(ptr).pure[F].widen[Ptr[_]]
    case InvalidPtrType =>
      FE.raiseError[Ptr[_]] {
        new Exception("Invalid native pointer being converted to MetaCall pointer")
      }
  }

  def toValue[F[_]](ptr: Ptr[_])(implicit
      FE: ApplicativeError[F, Throwable]
  ): F[Value] = ptr match {
    case p: BoolPtr   => Get[Boolean].value[F](p)
    case p: CharPtr   => Get[Char].value[F](p)
    case p: ShortPtr  => Get[Short].value[F](p)
    case p: IntPtr    => Get[Int].value[F](p)
    case p: LongPtr   => Get[Long].value[F](p)
    case p: FloatPtr  => Get[Float].value[F](p)
    case p: DoublePtr => Get[Double].value[F](p)
    case p: StringPtr => Get[String].value[F](p)
    case p: ArrayPtr  => Get[Array[Pointer]].value[F](p)
    case p: MapPtr    => Get[Array[(Pointer, Pointer)]].value[F](p)
    case p: NullPtr   => Get[Null].value[F](p)
    case p: SizePtr   => Get[SizeT].value[F](p)
  }

}

/** Represents pointer types
  * See https://github.com/metacall/core/blob/develop/source/metacall/include/metacall/metacall_value.h#L44
  * Unimplemented MetaCall types:
  * ```c
  * enum metacall_value_id {
  *   ...
  *   METACALL_BUFFER = 8,
  *  ...
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
object PtrType {
  def of(ptr: Pointer): PtrType =
    if (isNull(ptr)) InvalidPtrType
    else
      Bindings.instance.metacall_value_id(ptr) match {
        case 0  => BoolPtrType
        case 1  => CharPtrType
        case 2  => ShortPtrType
        case 3  => IntPtrType
        case 4  => LongPtrType
        case 5  => FloatPtrType
        case 6  => DoublePtrType
        case 7  => StringPtrType
        case 9  => ArrayPtrType
        case 10 => MapPtrType
        case 14 => NullPtrType
        case 17 => SizePtrType
        case _  => InvalidPtrType
      }
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

private[metacall] final class MapPtr(val ptr: Pointer)
    extends Ptr[Array[(Pointer, Pointer)]] {
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
