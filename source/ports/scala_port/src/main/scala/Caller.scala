package metacall

import metacall.util._
import java.util.concurrent.{LinkedBlockingQueue, ConcurrentHashMap}
import java.util.concurrent.atomic.{AtomicInteger, AtomicBoolean}

class Caller(scripts: List[Script]) {
  private case class Call(fnName: String, args: List[Value])

  private case class UniqueCall(call: Call, id: Int)

  private val callerThread = new Thread(() => {
    Bindings.instance.metacall_initialize()

    val loadResults = scripts.map { script =>
      script.filePath -> Bindings.instance.metacall_load_from_file(
        script.runtime.toString(),
        Array(script.filePath),
        SizeT(1),
        null
      )
    }

    loadResults.foreach { case (path, resultCode) =>
      if (resultCode != 0)
        throw new Exception("Failed to load script " + path)
    }

    while (!closed.get) {
      if (!callQueue.isEmpty()) {
        val uniqueCall = callQueue.take()
        val result = callUnsafe(uniqueCall.call.fnName, uniqueCall.call.args)
        callResultMap.put(uniqueCall.id, result)
      }
    }

    Bindings.instance.metacall_destroy()
  })

  private val closed = new AtomicBoolean(false)
  private val callQueue = new LinkedBlockingQueue[UniqueCall]()
  private val callResultMap = new ConcurrentHashMap[Int, Value]()
  private val callCounter = new AtomicInteger(0)

  def start(): Unit = callerThread.start()

  def destroy(): Unit = closed.set(true)

  /** Calls a loaded function.
    * WARNING: Should only be used from within the caller thread.
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  private def callUnsafe(fnName: String, args: List[Value]): Value = {
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
  def callV(fnName: String, args: List[Value]): Value = {
    val call = Call(fnName, args)
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

  /** Calls a loaded function
    * @param fnName The name of the function to call
    * @param args A product (tuple, case class, single value) to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  def call[A](fnName: String, args: A)(implicit AA: Args[A]): Value =
    callV(fnName, AA.from(args))

}
