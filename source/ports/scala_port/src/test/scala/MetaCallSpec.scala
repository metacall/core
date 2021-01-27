package metacall

import com.sun.jna._
import java.nio.file.Paths
import org.scalatest.flatspec.AnyFlatSpec
import cats.implicits._, cats.effect._
import metacall.util._, metacall.instances._
import com.sun.jna.ptr.PointerByReference

class MetaCallSpec extends AnyFlatSpec {
  val metacall = Bindings.instance

  val scriptPaths = Array(
    Paths.get("./src/test/scala/scripts/main.py").toAbsolutePath.toString()
  )

  "MetaCall" should "initialize correctly" in {
    require(
      metacall.metacall_initialize() == 0,
      "MetaCall could not initialize"
    )
  }

  "MetaCall" should "load script successsfully" in {
    require(
      metacall.metacall_load_from_file(
        "py",
        scriptPaths,
        SizeT(scriptPaths.length.toLong),
        null
      ) == 0,
      "MetaCall failed to load the script"
    )
  }

  "MetaCall" should "successfully call function from loaded script and return correct value" in {
    // Create array of parameters
    val args = Array(
      metacall.metacall_value_create_int(3),
      metacall.metacall_value_create_int(5)
    )

    // Invoke the function
    val retPtr = metacall.metacallv_s(
      "hello_scala_from_python",
      args,
      SizeT(args.length.toLong)
    )

    // Note: Python uses longs, so it returns a long value
    val retValue = metacall.metacall_value_to_long(retPtr)

    assert(retValue == 8)

    // For avoiding conversion errors, it is possible to test against metacall_value_id,
    // or there is also a casting API for dealing with it.

    // Clear parameters
    args.foreach(metacall.metacall_value_destroy)

    // Clear return value
    metacall.metacall_value_destroy(retPtr)
  }

  "MetaCall" should "call functions from transitively imported scripts" in {
    val argPtr = metacall.metacall_value_create_int(42)
    val retPtr = metacall.metacallv_s("imported_fn", Array(argPtr), SizeT(1))

    Ptr
      .fromPrimitive[IO](retPtr)
      .map(Ptr.toValue)
      .use { v =>
        assert(v == StringValue("Imported fn arg: 42"))
        IO.unit
      }
      .unsafeRunSync()

    metacall.metacall_value_destroy(argPtr)
    metacall.metacall_value_destroy(retPtr)
  }

  "Caller" should "call functions and clean up arguments and returned pointers" in {
    val ret = Caller
      .call[IO](
        "hello_scala_from_python",
        Vector(StringValue("Hello "), StringValue("Scala!"))
      )
      .unsafeRunSync()

    assert(ret == StringValue("Hello Scala!"))
  }

  "MetaCall" should "construct/parse maps correctly" in {
    val scalaMap = Map("one" -> 1, "two" -> 2, "three" -> 3)
    val mcTuples = scalaMap.toArray.map { case (k, v) =>
      metacall.metacall_value_create_array(
        Array(
          metacall.metacall_value_create_string(
            k,
            SizeT(k.getBytes().length.toLong)
          ),
          metacall.metacall_value_create_int(v)
        ),
        SizeT(2)
      )
    }

    val mcMapPtr =
      metacall.metacall_value_create_map(
        mcTuples,
        SizeT(mcTuples.length.toLong)
      )

    val mcMapValueId = metacall.metacall_value_id(mcMapPtr)
    assert(mcMapValueId == 10)

    val mcMapPtrSize = metacall.metacall_value_count(mcMapPtr)
    assert(mcMapPtrSize.intValue() == 3)

    val mcMap: Array[Pointer] =
      metacall.metacall_value_to_map(mcMapPtr).take(mcMapPtrSize.intValue())

    val scalaMapParsed = mcMap
      .map(pairPtr => metacall.metacall_value_to_array(pairPtr) -> pairPtr)
      .map { case (pair, ptr) =>
        pair.take(metacall.metacall_value_count(ptr).intValue())
      }
      .map {
        case Array(keyPtr, valuePtr) => {
          require(
            metacall.metacall_value_id(keyPtr) == 7,
            "Key vaue ID did not match MetaCall string ID"
          )
          require(
            metacall.metacall_value_id(valuePtr) == 3,
            "Key value ID did not match MetaCall int ID"
          )

          metacall.metacall_value_to_string(keyPtr) ->
            metacall.metacall_value_to_int(valuePtr)
        }
        case tuple =>
          fail(
            s"Array is supposed to be a tuple of 2, found tuple of ${tuple.length}"
          )
      }
      .toMap

    assert(scalaMap == scalaMapParsed)

    metacall.metacall_value_destroy(mcMapPtr)
  }

  "Int pointer creator/getter" should "work with IO" in {
    val intPtr = Ptr.from[Int, IO](22)
    val intGetter = implicitly[Get[Int]]
    intPtr
      .map(iptr => intGetter.primitive(iptr))
      .use { v =>
        IO(assert(v == 22))
      }
      .unsafeRunSync()
  }

  "Array pointer creator/getter" should "work" in {
    val elems = Vector("Hello", "from", "MetaCall!")
    val arrPtr = Ptr.fromVector[String, IO](elems)
    val arrayGetter = implicitly[Get[Array[Pointer]]]

    arrPtr
      .map(arrayGetter.primitive)
      .use { arr =>
        val newElems = arr.map(metacall.metacall_value_to_string).toVector
        IO(assert(newElems == elems))
      }
      .unsafeRunSync()
  }

  "Pointers" should "be created/retrieved correctly from Values" in {
    val values: List[Value] = List(
      IntValue(567),
      IntValue(Int.MaxValue),
      IntValue(Int.MinValue),
      FloatValue(11.22f),
      DoubleValue(1234.5678),
      DoubleValue(Double.MaxValue),
      DoubleValue(Double.MinValue),
      LongValue(1234567890),
      LongValue(Long.MaxValue),
      LongValue(Long.MinValue),
      StringValue("Helloooo"),
      CharValue('j'),
      StringValue("😍 🔥 ⚡"),
      BooleanValue(true),
      NullValue,
      ArrayValue(Vector(IntValue(1), StringValue("Hi"))),
      MapValue(
        Map(
          IntValue(1) -> IntValue(1),
          CharValue('2') -> FloatValue(2.0f),
          StringValue("3") -> LongValue(3L)
        )
      )
    )

    val valuePtrs = values.traverse(Ptr.fromValue[IO])

    val parsedValues = valuePtrs.map(_.map(Ptr.toValue))

    parsedValues
      .use { vs =>
        assert(values == vs)
        IO.unit
      }
      .unsafeRunSync()
  }

  "`FunctionPointer`s" should "be created/retrieved correctly" in {
    val cb = new FunctionPointer {
      override def callback(
          argc: SizeT,
          args: Pointer,
          data: Pointer
      ): Pointer = {
        val argsPtrArray = args.getPointerArray(0)

        metacall.metacall_value_copy(argsPtrArray.head)
      }
    }

    val fnRef = new PointerByReference()

    assert(
      metacall.metacall_registerv(
        null,
        cb,
        fnRef,
        StringPtrType.id,
        SizeT(1),
        Array(StringPtrType.id)
      ) == 0
    )

    val f = metacall.metacall_value_create_function(fnRef.getValue())

    val ret = metacall.metacallv_s(
      "apply_fn_to_str",
      Array(f),
      SizeT(1)
    )

    metacall.metacall_value_destroy(f)

    val res = metacall.metacall_value_to_string(ret)

    assert(res == "hellooo")

    metacall.metacall_value_destroy(ret)
  }

  "Function pointers" should "be constructed, passed, used, and destroyed correctly" in {
    val fnCallback = new FunctionPointer {
      final override def callback(
          argc: SizeT,
          args: Pointer,
          data: Pointer
      ): Pointer = {
        val argsPtrArray = args.getPointerArray(0)

        Ptr.toValue(Ptr.fromPrimitiveUnsafe(argsPtrArray.head)) match {
          case LongValue(l) => Ptr.fromValueUnsafe(LongValue(l + 3L)).ptr
          case _            => Ptr.fromValueUnsafe(NullValue).ptr
        }
      }
    }

    val fnRef = new PointerByReference()

    assert(
      metacall.metacall_registerv(
        null,
        fnCallback,
        fnRef,
        LongPtrType.id,
        SizeT(1),
        Array(LongPtrType.id)
      ) == 0
    )

    val fnPtr = fnRef.getValue()

    val ret = metacall.metacallv_s(
      "apply_fn_to_one",
      Array(metacall.metacall_value_create_function(fnPtr)),
      SizeT(1)
    )

    val res = metacall.metacall_value_to_long(ret)

    assert(res == 4L)

    metacall.metacall_value_destroy(ret)
  }

  "Function by parameters" should "retrieve the function, construct the value, call it and destroy it" in {
    val f = metacall.metacall_function("get_function_test")
    val v = metacall.metacall_value_create_function(f)
    val ret = metacall.metacallv_s(
      "apply_fn_to_one",
      Array(v),
      SizeT(1)
    )

    assert(metacall.metacall_value_to_long(ret) == 1L)

    metacall.metacall_value_destroy(ret)
    metacall.metacall_value_destroy(v)
  }

  "Function Create instance" should "create valid function pointers" in {
    val fnPtr = functionCreate.create {
      new FunctionPointer {
        def callback(argc: SizeT, args: Pointer, data: Pointer): Pointer = {
          val argPtrs =
            args.getPointerArray(0, argc.intValue()).map(Ptr.fromPrimitiveUnsafe)
          println("Getting arg values")
          val argValues = argPtrs.map(Ptr.toValue).toList

          argValues match {
            case StringValue(s) :: Nil =>
              Ptr.fromValueUnsafe(StringValue("Hello, " + s)).ptr
            case _ => Bindings.instance.metacall_value_create_null()
          }
        }
      }
    }

    val fnValue = functionGet.value(fnPtr).asInstanceOf[FunctionValue]

    val ret = fnValue.value(StringValue("World!") :: Nil)

    assert(ret == StringValue("Hello, World!"))
  }

  "FunctionValues" should "be constructed and passed to foreign functions" in {
    val fnVal = FunctionValue {
      case LongValue(l) :: Nil => LongValue(l + 1L)
      case _                   => NullValue
    }

    // TODO: This test causes segmentation fault
    val ret = Caller.call[IO]("apply_fn_to_one", Vector(fnVal)).unsafeRunSync()

    assert(ret == LongValue(2L))
  }

  "MetaCall" should "be destroyed successfully" in {
    require(
      metacall.metacall_destroy() == 0,
      "MetaCall was not successfully destroyed"
    )
  }
}
