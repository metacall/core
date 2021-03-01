package metacall

import metacall.instances._
import org.scalatest.flatspec.AnyFlatSpec
import concurrent.{Await, Future}, concurrent.duration._
import concurrent.ExecutionContext.Implicits.global

class CallerSpecRunner {
  def run() = {
    println("Executing CallerSpec Tests")
    (new CallerSpec()).execute()
  }
}

class CallerSpec extends AnyFlatSpec {

  private def await[A](f: Future[A]): A =
    Await.result(f, 2.seconds)

  "Caller" should "start successfully" in {
    println(
      s"----------------------- MetaCall started in ${ProcessHandle.current().pid()} -----------------------"
    )

    Caller.start(concurrent.ExecutionContext.global)
  }

  "Caller" should "load scripts into global scope successfully" in {
    await(Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/main.py"))
    await(Caller.loadFile(Runtime.Node, "./src/test/scala/scripts/main.js"))
  }

  "Caller" should "load scripts into namespaces and call them" in {
    await(Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/s1.py", "s1"))
    await(Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/s2.py", "s2"))

    assert(
      await(Caller.call("fn_in_s1", (), Some("s1"))) ==
        StringValue("Hello from s1")
    )

    assert(
      await(Caller.call("fn_in_s2", (), Some("s2"))) ==
        StringValue("Hello from s2")
    )
  }

  "Caller#definitions" should "retrieve functions correctly" in {
    val s1 = Caller.definitions("s1").get
    assert(
      s1 === Map(
        "fn_in_s1" -> Caller.FunctionMetadata(false, 0),
        "other_fn_in_s1" -> Caller.FunctionMetadata(false, 2)
      )
    )

    val s2 = Caller.definitions("s2").get
    assert(s2 === Map("fn_in_s2" -> Caller.FunctionMetadata(false, 0)))
  }

  "Caller" should "call functions and clean up arguments and returned pointers" in {
    val ret = await {
      Caller.callV(
        "hello_scala_from_python",
        List(StringValue("Hello "), StringValue("Scala!"))
      )
    }

    assert(ret == StringValue("Hello Scala!"))
  }

  "Caller" should "call async functions correctly" in {
    assert(
      await(Caller.call("sleep", 100)) ==
        StringValue("Slept 100 milliseconds!")
    )

    assert(
      await(Caller.call("sleepReturningPromise", 100)) ==
        StringValue("Slept 100 milliseconds!")
    )
  }

  "FunctionValues" should "be constructed and passed to foreign functions" in {
    val fnVal = FunctionValue {
      case LongValue(l) :: Nil => LongValue(l + 1L)
      case _                   => NullValue
    }

    val ret = await(Caller.callV("apply_fn_to_one", fnVal :: Nil))

    assert(ret == LongValue(2L))
  }

  "Generic API" should "operate on primitive Scala values" in {
    //  with tuples
    val ret = await(Caller.call("big_fn", (1, "hello", 2.2)))
    assert(ret == DoubleValue(8.2))

    // with single-element products (i.e. the List)
    val ret2 = await(Caller.call("sumList", List(1, 2, 3)))
    assert(ret2 == LongValue(6))

    // with HLists
    import shapeless._

    val ret3 = await(Caller.call("big_fn", 1 :: "hello" :: 2.2 :: HNil))
    assert(ret3 == DoubleValue(8.2))
  }

  "Using `Caller` from multiple threads" should "work" in {
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
