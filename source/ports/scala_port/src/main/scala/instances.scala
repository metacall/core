package metacall

import cats._, cats.implicits._
import metacall.util._

object instances {

  implicit val intCreate =
    new Create[Int, IntPtr] {
      def create[F[_]](
          value: Int
      )(implicit FE: ApplicativeError[F, Throwable]): F[IntPtr] = {
        val ptr = Bindings.instance.metacall_value_create_int(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new IntPtr(ptr).pure[F]
      }
    }

  implicit val intGet = new Get[Int, IntPtr] {
    def get[F[_]](ptr: IntPtr)(implicit FE: ApplicativeError[F, Throwable]): F[Int] =
      Bindings.instance.metacall_value_to_int(ptr.ptr).pure[F]
  }

  implicit val stringCreate = new Create[String, StringPtr] {
    def create[F[_]](
        value: String
    )(implicit FE: ApplicativeError[F, Throwable]): F[StringPtr] =
      new StringPtr(
        Bindings.instance.metacall_value_create_string(
          value,
          SizeT(value.getBytes().length.toLong)
        )
      ).pure[F]
  }

  implicit val stringGet = new Get[String, StringPtr] {
    def get[F[_]](
        ptr: StringPtr
    )(implicit FE: ApplicativeError[F, Throwable]): F[String] =
      Bindings.instance.metacall_value_to_string(ptr.ptr).pure[F]

  }

}
