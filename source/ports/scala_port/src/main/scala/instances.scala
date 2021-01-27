package metacall

import cats.implicits._
import com.sun.jna._
import metacall.util._
import com.sun.jna.ptr.PointerByReference

object instances {

  implicit val nullCreate =
    new Create[Null] {
      def create(value: Null): Ptr[Null] = {
        val ptr = Bindings.instance.metacall_value_create_null()
        new NullPtr(ptr)
      }
    }

  implicit val nullGet = new Get[Null] {
    def primitive(ptr: Ptr[Null]): Null =
      Bindings.instance.metacall_value_to_null(ptr.ptr)

    def value(ptr: Ptr[Null]): Value = NullValue
  }

  implicit val intCreate =
    new Create[Int] {
      def create(value: Int): Ptr[Int] = {
        val ptr = Bindings.instance.metacall_value_create_int(value)
        new IntPtr(ptr)
      }
    }

  implicit val intGet = new Get[Int] {
    def primitive(ptr: Ptr[Int]): Int =
      Bindings.instance.metacall_value_to_int(ptr.ptr)

    def value(ptr: Ptr[Int]): Value = IntValue(primitive(ptr))
  }

  implicit val longCreate =
    new Create[Long] {
      def create(value: Long): Ptr[Long] = {
        val ptr = Bindings.instance.metacall_value_create_long(value)
        new LongPtr(ptr)
      }
    }

  implicit val longGet = new Get[Long] {
    def primitive(ptr: Ptr[Long]): Long =
      Bindings.instance.metacall_value_to_long(ptr.ptr)

    def value(ptr: Ptr[Long]): Value = LongValue(primitive(ptr))
  }

  implicit val shortCreate =
    new Create[Short] {
      def create(value: Short): Ptr[Short] = {
        val ptr = Bindings.instance.metacall_value_create_short(value)
        new ShortPtr(ptr)
      }
    }

  implicit val shortGet = new Get[Short] {
    def primitive(ptr: Ptr[Short]): Short =
      Bindings.instance.metacall_value_to_short(ptr.ptr)

    def value(ptr: Ptr[Short]): Value = ShortValue(primitive(ptr))
  }

  implicit val floatCreate =
    new Create[Float] {
      def create(value: Float): Ptr[Float] = {
        val ptr = Bindings.instance.metacall_value_create_float(value)
        new FloatPtr(ptr)
      }
    }

  implicit val floatGet = new Get[Float] {
    def primitive(ptr: Ptr[Float]): Float =
      Bindings.instance.metacall_value_to_float(ptr.ptr)

    def value(ptr: Ptr[Float]): Value = FloatValue(primitive(ptr))
  }

  implicit val doubleCreate =
    new Create[Double] {
      def create(value: Double): Ptr[Double] = {
        val ptr = Bindings.instance.metacall_value_create_double(value)
        new DoublePtr(ptr)
      }
    }

  implicit val doubleGet = new Get[Double] {
    def primitive(ptr: Ptr[Double]): Double =
      Bindings.instance.metacall_value_to_double(ptr.ptr)

    def value(ptr: Ptr[Double]): Value = DoubleValue(primitive(ptr))
  }

  implicit val boolCreate =
    new Create[Boolean] {
      def create(value: Boolean): Ptr[Boolean] = {
        val ptr = Bindings.instance.metacall_value_create_bool(value)
        new BoolPtr(ptr)
      }
    }

  implicit val boolGet = new Get[Boolean] {
    def primitive(ptr: Ptr[Boolean]): Boolean =
      Bindings.instance.metacall_value_to_bool(ptr.ptr)

    def value(ptr: Ptr[Boolean]): Value = BooleanValue(primitive(ptr))
  }

  implicit val charCreate =
    new Create[Char] {
      def create(value: Char): Ptr[Char] = {
        val ptr = Bindings.instance.metacall_value_create_char(value)
        new CharPtr(ptr)
      }
    }

  implicit val charGet = new Get[Char] {
    def primitive(ptr: Ptr[Char]): Char =
      Bindings.instance.metacall_value_to_char(ptr.ptr)

    def value(ptr: Ptr[Char]): Value = CharValue(primitive(ptr))
  }

  implicit val stringCreate = new Create[String] {
    def create(value: String): Ptr[String] =
      new StringPtr(
        Bindings.instance.metacall_value_create_string(
          value,
          SizeT(value.getBytes().length.toLong)
        )
      )
  }

  implicit val stringGet = new Get[String] {
    def primitive(ptr: Ptr[String]): String =
      Bindings.instance.metacall_value_to_string(ptr.ptr)

    def value(ptr: Ptr[String]): Value = StringValue(primitive(ptr))
  }

  implicit val arrayCreate = new Create[Array[Pointer]] {
    def create(value: Array[Pointer]): Ptr[Array[Pointer]] =
      new ArrayPtr(
        Bindings.instance
          .metacall_value_create_array(value, SizeT(value.length.toLong))
      )
  }

  implicit val arrayGet = new Get[Array[Pointer]] {
    def primitive(ptr: Ptr[Array[Pointer]]): Array[Pointer] = {
      val dataSize = Bindings.instance.metacall_value_count(ptr.ptr)
      Bindings.instance.metacall_value_to_array(ptr.ptr).take(dataSize.intValue())
    }

    def value(ptr: Ptr[Array[Pointer]]): Value = {
      val elems = primitive(ptr).map(p => Ptr.toValue(Ptr.fromPrimitiveUnsafe(p)))
      ArrayValue(elems.toVector)
    }

  }

  implicit val mapCreate = new Create[Array[(Pointer, Pointer)]] {
    def create(value: Array[(Pointer, Pointer)]): Ptr[Array[(Pointer, Pointer)]] =
      new MapPtr(
        Bindings.instance
          .metacall_value_create_map(
            value.map { pair =>
              Bindings.instance
                .metacall_value_create_array(Array(pair._1, pair._2), SizeT(2))
            },
            SizeT(value.length.toLong)
          )
      )
  }

  implicit val mapGet = new Get[Array[(Pointer, Pointer)]] {
    def primitive(ptr: Ptr[Array[(Pointer, Pointer)]]): Array[(Pointer, Pointer)] = {
      val dataSize = Bindings.instance.metacall_value_count(ptr.ptr)
      val tuplePtrs =
        Bindings.instance.metacall_value_to_map(ptr.ptr).take(dataSize.intValue())
      tuplePtrs
        .map(Bindings.instance.metacall_value_to_array)
        .map(_.take(2))
        .map {
          case Array(k, v) => (k, v)
          case _ =>
            throw new Exception(
              "Map element is not an array of two elements. This is likey a bug, please report it."
            )
        }
    }

    def value(ptr: Ptr[Array[(Pointer, Pointer)]]): Value =
      MapValue {
        primitive(ptr).map { case (kPtr, vPtr) =>
          Ptr.toValue(Ptr.fromPrimitiveUnsafe(kPtr)) ->
            Ptr.toValue(Ptr.fromPrimitiveUnsafe(vPtr))
        }.toMap
      }
  }

  implicit val functionCreate = new Create[FunctionPointer] {
    def create(value: FunctionPointer): Ptr[FunctionPointer] = {
      val ref = new PointerByReference()

      if (Bindings.instance.metacall_registerv(
        null,
        value,
        ref,
        InvalidPtrType.id,
        SizeT(0),
        Array()
      ) != 0) {
        throw new Exception(
          "Invalid function value creation."
        )
      }

      new FunctionPtr(Bindings.instance.metacall_value_create_function(ref.getValue()))
    }
  }

  implicit val functionGet = new Get[FunctionPointer] {
    def primitive(ptr: Ptr[FunctionPointer]): FunctionPointer = {
      new FunctionPointer {
        def callback(
            argc: util.SizeT,
            args: Pointer,
            data: Pointer
        ): Pointer = {
          val fnPointer = Bindings.instance.metacall_value_to_function(ptr.ptr)

          Bindings.instance.metacallfv_s(
            fnPointer,
            args.getPointerArray(0),
            argc
          )
        }
      }
    }

    def value(ptr: Ptr[FunctionPointer]): Value = {
      val valueFn = (args: List[Value]) => {
        val argPtrArray = args.map(arg => Ptr.fromValueUnsafe(arg).ptr).toArray
        val fnPointer = Bindings.instance.metacall_value_to_function(ptr.ptr)
        val callbackRet =
          Bindings.instance.metacallfv_s(fnPointer, argPtrArray, SizeT(argPtrArray.size.asInstanceOf[Long]))
        val retPtr = Ptr.fromPrimitiveUnsafe(callbackRet)
        val retValue = Ptr.toValue(retPtr)

        Bindings.instance.metacall_value_destroy(callbackRet)

        for (argPtr <- argPtrArray) {
          Bindings.instance.metacall_value_destroy(argPtr)
        }

        retValue
      }

      FunctionValue(valueFn)
    }
  }

  implicit val invalidCreate = new Create[Unit] {
    def create(value: Unit): Ptr[Unit] = InvalidPtr
  }
}
