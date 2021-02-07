package metacall

import java.util.concurrent.ConcurrentHashMap
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.atomic.AtomicBoolean
import java.util.concurrent.atomic.AtomicInteger

import metacall.util._
import scala.concurrent.Future
import scala.concurrent.ExecutionContext

/** `Caller` creates a new thread on which:
  *   - a MetaCall instance is initialized (`Caller.start`)
  *   - Scripts are loaded (`Caller.loadFile`)
  *   - MetaCall functions are invoked (`Caller.call`)
  *
  *  You must always call `Caller.destroy` after you're done with it. This destroys
  *  the MetaCall instance.
  *
  *  Usage:
  *  ```scala
  *  Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/main.py")
  *  Caller.start()
  *  val ret = Caller.call("big_fn", (1, "hello", 2.2))
  *  assert(ret == DoubleValue(8.2))
  *  ```
  */
object Caller {
  private case class Call(namespace: Option[String], fnName: String, args: List[Value])

  private case class UniqueCall(call: Call, id: Int)

  private def callLoop() = {
    if (System.getProperty("java.polyglot.name") != "metacall")
      Bindings.instance.metacall_initialize()

    while (!closed.get) {
      if (!scriptsQueue.isEmpty()) {
        val script = scriptsQueue.take()
        Loader.loadFileUnsafe(script.runtime, script.filePath)
      } else if (!callQueue.isEmpty()) {
        val UniqueCall(Call(namespace, fnName, args), id) = callQueue.take()
        val result = callUnsafe(namespace, fnName, args)
        callResultMap.put(id, result)
      } else ()
    }

    if (System.getProperty("java.polyglot.name") != "metacall")
      Bindings.instance.metacall_destroy()
  }

  private val closed = new AtomicBoolean(false)
  private val callQueue = new LinkedBlockingQueue[UniqueCall]()
  private val callResultMap = new ConcurrentHashMap[Int, Value]()
  private val callCounter = new AtomicInteger(0)
  private val scriptsQueue = new LinkedBlockingQueue[Script]()

  def loadFile(runtime: Runtime, filePath: String, namespace: Option[String]): Unit =
    scriptsQueue.put(Script(filePath, runtime, namespace))

  def loadFile(runtime: Runtime, filePath: String): Unit =
    loadFile(runtime, filePath, None)

  def loadFile(runtime: Runtime, filePath: String, namespace: String): Unit =
    loadFile(runtime, filePath, Some(namespace))

  def start(): Unit = {
    if (System.getProperty("java.polyglot.name") != "metacall")
      new Thread(() => callLoop()).start()
    else
      callLoop()
  }

  def destroy(): Unit = closed.set(true)

  /** Calls a loaded function.
    * WARNING: Should only be used from within the caller thread.
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  private def callUnsafe(
      namespace: Option[String],
      fnName: String,
      args: List[Value]
  ): Value = {
    val argPtrArray = args.map(Ptr.fromValueUnsafe(_).ptr).toArray

    val retPointer =
      Bindings.instance.metacallv_s(fnName, argPtrArray, SizeT(argPtrArray.length.toLong))

    val retValue = Ptr.toValue(Ptr.fromPrimitiveUnsafe(retPointer))

    Bindings.instance.metacall_value_destroy(retPointer)
    argPtrArray.foreach(Bindings.instance.metacall_value_destroy)

    retValue
  }

  /** Calls a loaded function.
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  def callV(namespace: Option[String], fnName: String, args: List[Value]): Value = {
    val call = Call(namespace, fnName, args)
    val callId = callCounter.get + 1

    if (callId == Int.MaxValue)
      callCounter.set(0)
    else
      callCounter.set(callId)

    val uniqueCall = UniqueCall(call, callId)

    callQueue.put(uniqueCall)

    var result: Value = null

    while (result == null)
      result = callResultMap.get(callId)

    callResultMap.remove(callId)

    result
  }

  def call[A](namespace: Option[String], fnName: String, args: A)(implicit
      AA: Args[A],
      ec: ExecutionContext
  ): Future[Value] =
    Future { blocking.call[A](namespace, fnName, args) }

  def call[A](fnName: String, args: A)(implicit
      AA: Args[A],
      ec: ExecutionContext
  ): Future[Value] =
    call[A](None, fnName, args)

  def call[A](namespace: String, fnName: String, args: A)(implicit
      AA: Args[A],
      ec: ExecutionContext
  ): Future[Value] =
    call[A](Some(namespace), fnName, args)
  object blocking {

    /** Calls a loaded function
      * @param fnName The name of the function to call
      * @param args A product (tuple, case class, single value) to be passed as arguments to the function
      * @return The function's return value, or `InvalidValue` in case of an error
      */
    def call[A](namespace: Option[String], fnName: String, args: A)(implicit
        AA: Args[A]
    ): Value =
      callV(namespace, fnName, AA.from(args))

    def call[A](fnName: String, args: A)(implicit AA: Args[A]): Value =
      call[A](None, fnName, args)

    def call[A](namespace: String, fnName: String, args: A)(implicit AA: Args[A]): Value =
      call[A](Some(namespace), fnName, args)

  }

}
