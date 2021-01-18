package metacall

import cats._, cats.implicits._
import com.sun.jna._
import metacall.util._

object instances {

  implicit val sizeCreate =
    new Create[SizeT] {
      def create[F[_]](
          value: SizeT
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[SizeT]] = {
        val ptr = Bindings.instance.metacall_value_create_long(value.longValue())

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(
              Some(value.longValue()),
              Some("Pointer to value was null")
            )
          )
        else new SizePtr(ptr).pure[F].widen[Ptr[SizeT]]
      }
    }

  implicit val sizeGet = new Get[SizeT] {
    def primitive[F[_]](ptr: Ptr[SizeT])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[SizeT] =
      SizeT(Bindings.instance.metacall_value_to_long(ptr.ptr)).pure[F]

    def value[F[_]](ptr: Ptr[SizeT])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(p => SizeTValue(p.longValue()))
  }

  implicit val nullCreate =
    new Create[Null] {
      def create[F[_]](
          value: Null
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Null]] = {
        val ptr = Bindings.instance.metacall_value_create_null()

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(None, Some("Pointer to value was null"))
          )
        else new NullPtr(ptr).pure[F].widen[Ptr[Null]]
      }
    }

  implicit val nullGet = new Get[Null] {
    def primitive[F[_]](ptr: Ptr[Null])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Null] =
      Applicative[F].pure(Bindings.instance.metacall_value_to_null(ptr.ptr))

    def value[F[_]](ptr: Ptr[Null])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = NullValue.pure[F].widen[Value]
  }

  implicit val intCreate =
    new Create[Int] {
      def create[F[_]](
          @specialized value: Int
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Int]] = {
        val ptr = Bindings.instance.metacall_value_create_int(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new IntPtr(ptr).pure[F].widen[Ptr[Int]]
      }
    }

  implicit val intGet = new Get[Int] {
    def primitive[F[_]](ptr: Ptr[Int])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Int] =
      Bindings.instance.metacall_value_to_int(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Int])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(IntValue.apply)
  }

  implicit val longCreate =
    new Create[Long] {
      def create[F[_]](
          value: Long
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Long]] = {
        val ptr = Bindings.instance.metacall_value_create_long(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new LongPtr(ptr).pure[F].widen[Ptr[Long]]
      }
    }

  implicit val longGet = new Get[Long] {
    def primitive[F[_]](ptr: Ptr[Long])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Long] =
      Bindings.instance.metacall_value_to_long(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Long])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(LongValue.apply)
  }

  implicit val shortCreate =
    new Create[Short] {
      def create[F[_]](
          value: Short
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Short]] = {
        val ptr = Bindings.instance.metacall_value_create_short(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new ShortPtr(ptr).pure[F].widen[Ptr[Short]]
      }
    }

  implicit val shortGet = new Get[Short] {
    def primitive[F[_]](ptr: Ptr[Short])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Short] =
      Bindings.instance.metacall_value_to_short(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Short])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(ShortValue.apply)
  }

  implicit val floatCreate =
    new Create[Float] {
      def create[F[_]](
          value: Float
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Float]] = {
        val ptr = Bindings.instance.metacall_value_create_float(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new FloatPtr(ptr).pure[F].widen[Ptr[Float]]
      }
    }

  implicit val floatGet = new Get[Float] {
    def primitive[F[_]](ptr: Ptr[Float])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Float] =
      Bindings.instance.metacall_value_to_float(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Float])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(FloatValue.apply)
  }

  implicit val doubleCreate =
    new Create[Double] {
      def create[F[_]](
          value: Double
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Double]] = {
        val ptr = Bindings.instance.metacall_value_create_double(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new DoublePtr(ptr).pure[F].widen[Ptr[Double]]
      }
    }

  implicit val doubleGet = new Get[Double] {
    def primitive[F[_]](ptr: Ptr[Double])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Double] =
      Bindings.instance.metacall_value_to_double(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Double])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(DoubleValue.apply)
  }

  implicit val boolCreate =
    new Create[Boolean] {
      def create[F[_]](
          value: Boolean
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Boolean]] = {
        val ptr = Bindings.instance.metacall_value_create_bool(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new BoolPtr(ptr).pure[F].widen[Ptr[Boolean]]
      }
    }

  implicit val boolGet = new Get[Boolean] {
    def primitive[F[_]](ptr: Ptr[Boolean])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Boolean] =
      Bindings.instance.metacall_value_to_bool(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Boolean])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(BooleanValue.apply)
  }

  implicit val charCreate =
    new Create[Char] {
      def create[F[_]](
          value: Char
      )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Char]] = {
        val ptr = Bindings.instance.metacall_value_create_char(value)

        if (isNull(ptr))
          FE.raiseError(
            new AllocationError(Some(value), Some("Pointer to value was null"))
          )
        else new CharPtr(ptr).pure[F].widen[Ptr[Char]]
      }
    }

  implicit val charGet = new Get[Char] {
    def primitive[F[_]](ptr: Ptr[Char])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Char] =
      Bindings.instance.metacall_value_to_char(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[Char])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(CharValue.apply)
  }

  implicit val stringCreate = new Create[String] {
    def create[F[_]](
        value: String
    )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[String]] =
      new StringPtr(
        Bindings.instance.metacall_value_create_string(
          value,
          SizeT(value.getBytes().length.toLong)
        )
      ).pure[F].widen[Ptr[String]]
  }

  implicit val stringGet = new Get[String] {
    def primitive[F[_]](
        ptr: Ptr[String]
    )(implicit FE: ApplicativeError[F, Throwable]): F[String] =
      Bindings.instance.metacall_value_to_string(ptr.ptr).pure[F]

    def value[F[_]](ptr: Ptr[String])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = primitive[F](ptr).map(StringValue.apply)
  }

  implicit val arrayCreate = new Create[Array[Pointer]] {
    def create[F[_]](
        value: Array[Pointer]
    )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Array[Pointer]]] =
      new ArrayPtr(
        Bindings.instance
          .metacall_value_create_array(value, SizeT(value.length.toLong))
      ).pure[F].widen[Ptr[Array[Pointer]]]
  }

  implicit val arrayGet = new Get[Array[Pointer]] {
    def primitive[F[_]](
        ptr: Ptr[Array[Pointer]]
    )(implicit FE: ApplicativeError[F, Throwable]): F[Array[Pointer]] = {
      val dataSize = Bindings.instance.metacall_value_count(ptr.ptr)
      Bindings.instance.metacall_value_to_array(ptr.ptr).take(dataSize.intValue()).pure[F]
    }

    def value[F[_]](ptr: Ptr[Array[Pointer]])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = ???
  }

  implicit val mapCreate = new Create[Array[(Pointer, Pointer)]] {
    def create[F[_]](
        value: Array[(Pointer, Pointer)]
    )(implicit FE: ApplicativeError[F, Throwable]): F[Ptr[Array[(Pointer, Pointer)]]] =
      new MapPtr(
        Bindings.instance
          .metacall_value_create_map(
            value.map { pair =>
              Bindings.instance
                .metacall_value_create_array(Array(pair._1, pair._2), SizeT(2))
            },
            SizeT(value.length.toLong)
          )
      ).pure[F].widen[Ptr[Array[(Pointer, Pointer)]]]
  }

  implicit val mapGet = new Get[Array[(Pointer, Pointer)]] {
    def primitive[F[_]](
        ptr: Ptr[Array[(Pointer, Pointer)]]
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

    def value[F[_]](ptr: Ptr[Array[(Pointer, Pointer)]])(implicit
        FE: ApplicativeError[F, Throwable]
    ): F[Value] = ???
  }

}
