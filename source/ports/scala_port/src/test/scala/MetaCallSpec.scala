package metacall

import com.sun.jna._
import java.nio.file.Paths
import org.scalatest.flatspec.AnyFlatSpec
import cats.implicits._, cats.effect._
import metacall.util._, metacall.instances._

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
      .evalMap(iptr => intGetter.get[IO](iptr))
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
      .evalMap(arrayGetter.get[IO])
      .use { arr =>
        val newElems = arr.map(metacall.metacall_value_to_string).toVector
        IO(assert(newElems == elems))
      }
      .unsafeRunSync()
  }

  "MetaCall" should "be destroyed successfully" in {
    require(
      metacall.metacall_destroy() == 0,
      "MetaCall was not successfully destroyed"
    )
  }

}
