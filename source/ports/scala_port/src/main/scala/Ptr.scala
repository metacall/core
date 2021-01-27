package metacall

import metacall.util._
import com.sun.jna._, ptr.PointerByReference
import cats._, cats.implicits._, cats.effect._

/** Create a [[Ptr]] to MetaCall value of type [[A]] */
trait Create[A] {
  def create(value: A): Ptr[A]
}
object Create {
  def apply[A](implicit C: Create[A]) = C
}

/** Gets the value of a [[Ptr]]. */
trait Get[A] {

  /** Get the primitive representation of the pointer's value */
  def primitive(ptr: Ptr[A]): A

  /** Get the pointer's high-level [[metacall.Value]] representation */
  def value(ptr: Ptr[A]): Value

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
      FE: MonadError[F, Throwable],
      FD: Defer[F],
      C: Create[A]
  ): Resource[F, Ptr[A]] =
    Resource.make(C.create(value).pure[F]) { vPtr =>
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
      FE: MonadError[F, Throwable],
      FD: Defer[F],
      CA: Create[A],
      CR: Create[Array[Pointer]]
  ): Resource[F, Ptr[Array[Pointer]]] = {
    val elemPtrs = vec.map(a => CA.create(a).ptr)

    Resource.suspend(from[Array[Pointer], F](elemPtrs.toArray).pure[F])
  }

  import metacall.instances._

  /** Returns an unmanaged pointer to the creted value. */
  private[metacall] def fromValueUnsafe(v: Value): Ptr[_] = v match {
    case CharValue(value)    => Create[Char].create(value)
    case StringValue(value)  => Create[String].create(value)
    case ShortValue(value)   => Create[Short].create(value)
    case IntValue(value)     => Create[Int].create(value)
    case LongValue(value)    => Create[Long].create(value)
    case FloatValue(value)   => Create[Float].create(value)
    case DoubleValue(value)  => Create[Double].create(value)
    case BooleanValue(value) => Create[Boolean].create(value)
    case InvalidValue        => Create[Unit].create(())
    case ArrayValue(value) => {
      val elemPtrs = value.map(fromValueUnsafe(_).ptr).toArray
      Create[Array[Pointer]].create(elemPtrs)
    }
    case MapValue(value) => {
      val tuplePtrs = value.toArray.map { case (k, v) =>
        fromValueUnsafe(k).ptr -> fromValueUnsafe(v).ptr
      }

      Create[Array[(Pointer, Pointer)]].create(tuplePtrs)
    }
    case FunctionValue(fn) =>
      Create[FunctionPointer].create {
        new FunctionPointer {
          def callback(argc: SizeT, arg: PointerByReference, data: Pointer): Pointer = {
            val argValue =
              Ptr.toValue(Ptr.fromPrimitiveUnsafe(arg.getValue()))

            Ptr.fromValueUnsafe(fn(argValue)).ptr
          }
        }
      }
    case NullValue => Create[Null].create(null)
  }

  def fromValue[F[_]](v: Value)(implicit
      FE: MonadError[F, Throwable],
      FD: Defer[F]
  ): Resource[F, Ptr[_]] = Resource.make(fromValueUnsafe(v).pure[F]) { vPtr =>
    FD.defer {
      try FE.pure(Bindings.instance.metacall_value_destroy(vPtr.ptr))
      catch {
        case e: Throwable =>
          FE.raiseError(new DestructionError(vPtr.ptr, Some(e.getMessage())))
      }
    }
  }

  /** Returns an unmanaged pointer that you need to destroy yourself,
    * or make sure it's destroyed down the line
    */
  private[metacall] def fromPrimitiveUnsafe(pointer: Pointer): Ptr[_] =
    PtrType.of(pointer) match {
      case BoolPtrType     => new BoolPtr(pointer)
      case CharPtrType     => new CharPtr(pointer)
      case ShortPtrType    => new ShortPtr(pointer)
      case IntPtrType      => new IntPtr(pointer)
      case LongPtrType     => new LongPtr(pointer)
      case FloatPtrType    => new FloatPtr(pointer)
      case DoublePtrType   => new DoublePtr(pointer)
      case StringPtrType   => new StringPtr(pointer)
      case ArrayPtrType    => new ArrayPtr(pointer)
      case MapPtrType      => new MapPtr(pointer)
      case NullPtrType     => new NullPtr(pointer)
      case FunctionPtrType => new FunctionPtr(pointer)
      case InvalidPtrType  => InvalidPtr
    }

  private[metacall] def fromPrimitive[F[_]](pointer: Pointer)(implicit
      FE: MonadError[F, Throwable],
      FD: Defer[F]
  ): Resource[F, Ptr[_]] =
    Resource
      .make(fromPrimitiveUnsafe(pointer).pure[F].widen[Ptr[_]]) { p =>
        FD.defer(FE.pure(Bindings.instance.metacall_value_count(p.ptr)))
      }

  def toValue(ptr: Ptr[_]): Value = ptr match {
    case p: BoolPtr     => Get[Boolean].value(p)
    case p: CharPtr     => Get[Char].value(p)
    case p: ShortPtr    => Get[Short].value(p)
    case p: IntPtr      => Get[Int].value(p)
    case p: LongPtr     => Get[Long].value(p)
    case p: FloatPtr    => Get[Float].value(p)
    case p: DoublePtr   => Get[Double].value(p)
    case p: StringPtr   => Get[String].value(p)
    case p: ArrayPtr    => Get[Array[Pointer]].value(p)
    case p: MapPtr      => Get[Array[(Pointer, Pointer)]].value(p)
    case p: NullPtr     => Get[Null].value(p)
    case p: FunctionPtr => Get[FunctionPointer].value(p)
    case InvalidPtr     => InvalidValue
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
        case 13 => FunctionPtrType
        case 14 => NullPtrType
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

private[metacall] final class FunctionPtr(val ptr: Pointer) extends Ptr[FunctionPointer] {
  val ptrType: PtrType = FunctionPtrType

  /** This reference is here just to keep the function ref from being garbage collected */
  private var ref: PointerByReference = null

  /** Don't forget to use this method when creating a new instance. */
  private[metacall] def setRef(ref: PointerByReference): Unit =
    if (this.ref == null) this.ref = ref
}
object FunctionPtrType extends PtrType {
  val id = 13
}

private[metacall] final class NullPtr(val ptr: Pointer) extends Ptr[Null] {
  val ptrType = NullPtrType
}
object NullPtrType extends PtrType {
  val id = 14
}

case object InvalidPtr extends Ptr[Unit] {
  val ptr = null
  val ptrType = InvalidPtrType
}
object InvalidPtrType extends PtrType {
  val id = 18
}
