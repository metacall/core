package metacall

import metacall.instances._
import org.scalatest.flatspec.AnyFlatSpec
import scala.util._

class CallerSpecRunner {
  def run() = {
    println("Executing CallerSpec Tests")
    (new CallerSpec()).execute()
  }
}

class CallerSpec extends AnyFlatSpec {
  "Caller" should "start successfully" in {
    println(
      s"----------------------- MetaCall started in ${ProcessHandle.current().pid()} -----------------------"
    )
    Caller.start()
  }

  "Caller" should "load scripts into global scope successfully" in {
    Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/main.py")
  }

  "Caller" should "load scripts into namespaces and call them" in {
    Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/s1.py", "s1")
    Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/s2.py", "s2")

    assert(
      Caller.blocking.call("fn_in_s1", (), Some("s1")) ==
        Success(StringValue("Hello from s1"))
    )
    assert(
      Caller.blocking.call("fn_in_s2", (), Some("s2")) ==
        Success(StringValue("Hello from s2"))
    )
  }

  "Caller" should "call functions and clean up arguments and returned pointers" in {
    val ret = Caller.blocking.callV(
      "hello_scala_from_python",
      List(StringValue("Hello "), StringValue("Scala!"))
    )

    assert(ret == Success(StringValue("Hello Scala!")))
  }

  "FunctionValues" should "be constructed and passed to foreign functions" in {
    val fnVal = FunctionValue {
      case LongValue(l) :: Nil => LongValue(l + 1L)
      case _                   => NullValue
    }

    val ret = Caller.blocking.callV("apply_fn_to_one", fnVal :: Nil).get

    assert(ret == LongValue(2L))
  }

  "Generic API" should "operate on primitive Scala values" in {
    //  with tuples
    val ret = Caller.blocking.call("big_fn", (1, "hello", 2.2)).get
    assert(ret == DoubleValue(8.2))

    // with single-element products (i.e. the List)
    val ret2 = Caller.blocking.call("sumList", List(1, 2, 3)).get
    assert(ret2 == LongValue(6))

    // with HLists
    import shapeless._

    val ret3 = Caller.blocking.call("big_fn", 1 :: "hello" :: 2.2 :: HNil).get
    assert(ret3 == DoubleValue(8.2))
  }

  "Using `Caller` from multiple threads" should "work" in {
    import scala.concurrent._, duration._
    import ExecutionContext.Implicits.global

    val rangeValues: List[ArrayValue] =
      List.range(1, 50).map(n => ArrayValue(Vector.range(1, n).map(IntValue)))

    val resSum = Future
      .traverse(rangeValues) { range =>
        Future(Caller.blocking.callV("sumList", range :: Nil)) map {
          case Success(n: NumericValue[_]) => n.long.value
          case other                       => fail("Returned value should be a number, but got " + other)
        }
      }
      .map(_.sum)

    val result = Await.result(resSum, 10.seconds)

    assert(result == 19600)
  }

  "Calling functions many times in parallel" should "work" in {
    import scala.concurrent._, duration._
    import ExecutionContext.Implicits.global

    val rangeValues: List[ArrayValue] =
      List.range(1, 50).map(n => ArrayValue(Vector.range(1, n).map(IntValue)))

    val resSum = Future
      .traverse(rangeValues) { range =>
        Caller.callV("sumList", range :: Nil) map {
          case n: NumericValue[_] => n.long.value
          case other              => fail("Returned value should be a number, but got " + other)
        }
      }
      .map(_.sum)

    val result = Await.result(resSum, 10.seconds)

    assert(result == 19600)
  }

  "Caller" should "be destroyed correctly" in {
    Caller.destroy()
  }
}
