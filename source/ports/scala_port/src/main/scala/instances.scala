package metacall

import cats._, cats.implicits._
import com.sun.jna._
import metacall.util._

object instances {

  implicit val sizeCreate =
    new Create[SizeT, SizePtr] {
      def create[F[_]](
          value: SizeT
      )(implicit FE: ApplicativeError[F, Throwable]): F[SizePtr] = {
        val ptr = Bindings.instance.metacall_value_create_long(value.longValue())

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(
              Some(value.longValue()),
              Some("Pointer to value was null")
            )
          )
        else new SizePtr(ptr).pure[F]
      }
    }

  implicit val sizeGet = new Get[SizeT, SizePtr] {
    def get[F[_]](ptr: SizePtr)(implicit FE: ApplicativeError[F, Throwable]): F[SizeT] =
      SizeT(Bindings.instance.metacall_value_to_long(ptr.ptr)).pure[F]
  }

  implicit val nullCreate =
    new Create[Null, NullPtr] {
      def create[F[_]](
          value: Null
      )(implicit FE: ApplicativeError[F, Throwable]): F[NullPtr] = {
        val ptr = Bindings.instance.metacall_value_create_null()

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(None, Some("Pointer to value was null"))
          )
        else new NullPtr(ptr).pure[F]
      }
    }

  implicit val nullGet = new Get[SizeT, SizePtr] {
    def get[F[_]](ptr: SizePtr)(implicit FE: ApplicativeError[F, Throwable]): F[SizeT] =
      Applicative[F].pure(Bindings.instance.metacall_value_to_null(ptr.ptr))
  }

  implicit val intCreate =
    new Create[Int, IntPtr] {
      def create[F[_]](
          @specialized value: Int
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

  implicit val longCreate =
    new Create[Long, LongPtr] {
      def create[F[_]](
          value: Long
      )(implicit FE: ApplicativeError[F, Throwable]): F[LongPtr] = {
        val ptr = Bindings.instance.metacall_value_create_long(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new LongPtr(ptr).pure[F]
      }
    }

  implicit val longGet = new Get[Long, LongPtr] {
    def get[F[_]](ptr: LongPtr)(implicit FE: ApplicativeError[F, Throwable]): F[Long] =
      Bindings.instance.metacall_value_to_long(ptr.ptr).pure[F]
  }

  implicit val shortCreate =
    new Create[Short, ShortPtr] {
      def create[F[_]](
          value: Short
      )(implicit FE: ApplicativeError[F, Throwable]): F[ShortPtr] = {
        val ptr = Bindings.instance.metacall_value_create_short(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new ShortPtr(ptr).pure[F]
      }
    }

  implicit val shortGet = new Get[Short, ShortPtr] {
    def get[F[_]](ptr: ShortPtr)(implicit FE: ApplicativeError[F, Throwable]): F[Short] =
      Bindings.instance.metacall_value_to_short(ptr.ptr).pure[F]
  }

  implicit val floatCreate =
    new Create[Float, FloatPtr] {
      def create[F[_]](
          value: Float
      )(implicit FE: ApplicativeError[F, Throwable]): F[FloatPtr] = {
        val ptr = Bindings.instance.metacall_value_create_float(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new FloatPtr(ptr).pure[F]
      }
    }

  implicit val floatGet = new Get[Float, FloatPtr] {
    def get[F[_]](ptr: FloatPtr)(implicit FE: ApplicativeError[F, Throwable]): F[Float] =
      Bindings.instance.metacall_value_to_float(ptr.ptr).pure[F]
  }

  implicit val doubleCreate =
    new Create[Double, DoublePtr] {
      def create[F[_]](
          value: Double
      )(implicit FE: ApplicativeError[F, Throwable]): F[DoublePtr] = {
        val ptr = Bindings.instance.metacall_value_create_double(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new DoublePtr(ptr).pure[F]
      }
    }

  implicit val doubleGet = new Get[Double, DoublePtr] {
    def get[F[_]](ptr: DoublePtr)(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Double] =
      Bindings.instance.metacall_value_to_double(ptr.ptr).pure[F]
  }

  implicit val boolCreate =
    new Create[Boolean, BoolPtr] {
      def create[F[_]](
          value: Boolean
      )(implicit FE: ApplicativeError[F, Throwable]): F[BoolPtr] = {
        val ptr = Bindings.instance.metacall_value_create_bool(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new BoolPtr(ptr).pure[F]
      }
    }

  implicit val boolGet = new Get[Boolean, BoolPtr] {
    def get[F[_]](ptr: BoolPtr)(implicit FE: ApplicativeError[F, Throwable]): F[Boolean] =
      Bindings.instance.metacall_value_to_bool(ptr.ptr).pure[F]
  }

  implicit val charCreate =
    new Create[Char, CharPtr] {
      def create[F[_]](
          value: Char
      )(implicit FE: ApplicativeError[F, Throwable]): F[CharPtr] = {
        val ptr = Bindings.instance.metacall_value_create_char(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new CharPtr(ptr).pure[F]
      }
    }

  implicit val charGet = new Get[Char, CharPtr] {
    def get[F[_]](ptr: CharPtr)(implicit FE: ApplicativeError[F, Throwable]): F[Char] =
      Bindings.instance.metacall_value_to_char(ptr.ptr).pure[F]
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

  implicit val arrayCreate = new Create[Array[Pointer], ArrayPtr] {
    def create[F[_]](
        value: Array[Pointer]
    )(implicit FE: ApplicativeError[F, Throwable]): F[ArrayPtr] =
      new ArrayPtr(
        Bindings.instance
          .metacall_value_create_array(value, SizeT(value.length.toLong))
      ).pure[F]
  }

  implicit val arrayGet = new Get[Array[Pointer], ArrayPtr] {
    def get[F[_]](
        ptr: ArrayPtr
    )(implicit FE: ApplicativeError[F, Throwable]): F[Array[Pointer]] = {
      val dataSize = Bindings.instance.metacall_value_count(ptr.ptr)
      Bindings.instance.metacall_value_to_array(ptr.ptr).take(dataSize.intValue()).pure[F]
    }
  }

  implicit val mapCreate = new Create[Array[(Pointer, Pointer)], MapPtr] {
    def create[F[_]](
        value: Array[(Pointer, Pointer)]
    )(implicit FE: ApplicativeError[F, Throwable]): F[MapPtr] =
      new MapPtr(
        Bindings.instance
          .metacall_value_create_map(
            value.map { pair =>
              Bindings.instance
                .metacall_value_create_array(Array(pair._1, pair._2), SizeT(2))
            },
            SizeT(value.length.toLong)
          )
      ).pure[F]
  }

  implicit val mapGet = new Get[Array[(Pointer, Pointer)], MapPtr] {
    def get[F[_]](
        ptr: MapPtr
    )(implicit FE: ApplicativeError[F, Throwable]): F[Array[(Pointer, Pointer)]] = {
      val dataSize = Bindings.instance.metacall_value_count(ptr.ptr)
      val tuplePtrs =
        Bindings.instance.metacall_value_to_map(ptr.ptr).take(dataSize.intValue())
      tuplePtrs.toVector
        .map(Bindings.instance.metacall_value_to_array)
        .traverse {
          case Array(k, v) => (k, v).pure[F]
          case _ =>
            FE.raiseError[(Pointer, Pointer)](new Exception("Tuple size wasn't two"))
        }
        .map(_.toArray)
    }
  }

}
