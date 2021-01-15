package metacall

import java.nio.file.Paths
import org.scalatest.flatspec.AnyFlatSpec

class MetaCallSpec extends AnyFlatSpec {
  val metacall = MetaCallBindings.instance

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
    val scalaMap = Map("one" -> 1, "two" -> 2)
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

    val mcMap = metacall.metacall_value_to_map(mcMapPtr)

    mcMap foreach (v => println(metacall.metacall_value_id(v)))

    val scalaMapParsed = mcMap
      .map(metacall.metacall_value_to_array)
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

  "MetaCall" should "be destroyed successfully" in {
    require(
      metacall.metacall_destroy() == 0,
      "MetaCall was not successfully destroyed"
    )
  }

}

// This is a experiment I have been doing in order to implement a high level
// abstraction for Scala. The objective is to make it as trasparent as possible,
// like if you were dealing with normal scala values. Generics can help for sure.

/*
  trait ValueLifetime {
    def finalize()
  }

  def finalizer [T, V <: ValueLifetime] (v : V) (f : V => T) : T =
    try f(v)
    finally v.finalize()

  abstract class ValuePtr[T](v: Pointer) extends ValueLifetime {
    def finalize() {
      metacall.metacall_value_destroy(v)
    }
  }

  class Value[@specialized(Int) T](i: Int) extends ValuePtr[T](metacall.metacall_value_create_int(i)) {
    def to_value(): T = {
      metacall.metacall_value_to_int(v)
    }
  }

  class Value[@specialized(String) T](str: String) extends ValuePtr(metacall.metacall_value_create_string(i)) {

  }
 */
