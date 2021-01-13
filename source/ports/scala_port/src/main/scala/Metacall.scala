package metacall

import com.sun.jna._

trait MetacallBindings extends Library {
  def metacall_load_from_file(
      runtime: String,
      paths: Array[String],
      size: Int,
      handle: Array[Pointer]
  ): Int

  def metacallv_s(
      functionName: String,
      args: Array[Pointer],
      argsSize: Int
  ): Pointer

  def metacall_initialize(): Int

  def metacall_value_create_string(str: String, byteSize: Int): Pointer

  def metacall_value_create_int(i: Int): Pointer

  def metacall_destroy(value: Pointer): Int
}

object Metacall {
  // CAUTION: Always check pointers passed to metacall (should not be Java null)

  val mc = Native.load("metacall", classOf[MetacallBindings])
  import mc._

  metacall_initialize()

  println(
    metacall_load_from_file(
      "node",
      Array("./src/test/scala/main.js"),
      1,
      Array.empty
    )
  ) // outputs 1

  // Call hello
  val arg = metacall_value_create_string("Jack", "Jack".getBytes().length)
  println("ARG: " + arg.getString(0)) // works!
  val r = metacallv_s("hello", Array(arg), 1)
  println("R1: " + r) // does not work...
  metacall_destroy(arg) // works!

  // Call increment
  val n = metacall_value_create_int(50)
  println("N: " + n.getInt(0)) // works!
  val r2 = metacallv_s("increment", Array(n), 1)
  println("R2: " + r2) // does not work...
}
