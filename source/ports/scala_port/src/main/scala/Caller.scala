package metacall

import metacall.util._
import java.util.concurrent.LinkedBlockingQueue
import java.util.concurrent.atomic.AtomicBoolean
import scala.collection.mutable.{Map => MutMap}

object Caller {

  private val closed = new AtomicBoolean(false)
  private val callsQueue = new LinkedBlockingQueue[UniqueCall]()
  private val returns = new Observable[(UniqueCall, Value)]()
  private val callCounts = MutMap.empty[Call, Int]

  def init(): Unit = {
    while (!closed.get) {
      val uniqueCall = callsQueue.take()
      if (uniqueCall != null) {
        returns.emit((uniqueCall, uniqueCall.call.invoke))
      }
    }
  }

  def destroy(): Unit = closed.set(true)

  /** Calls a loaded function
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  private[metacall] def callV(fnName: String, args: List[Value]): Value = {
    val argPtrArray = args.map(Ptr.fromValueUnsafe(_).ptr).toArray

    val retPointer =
      Bindings.instance.metacallv_s(fnName, argPtrArray, SizeT(argPtrArray.length.toLong))

    val retValue = Ptr.toValue(Ptr.fromPrimitiveUnsafe(retPointer))

    Bindings.instance.metacall_value_destroy(retPointer)
    argPtrArray.foreach(Bindings.instance.metacall_value_destroy)

    retValue
  }

  /** Calls a loaded function
    * @param fnName The name of the function to call
    * @param args A product (tuple, case class, single value) to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  def call[A](fnName: String, args: A)(implicit AA: Args[A]): Value = {

    val call = Call(fnName, AA.from(args))
    val callCount = callCounts(call) + 1
    callCounts.update(call, callCount) // update call count
    val uniqueCall = UniqueCall(call, callCount)

    callsQueue.put(uniqueCall)

    var result: Value = null

    returns.observe { case (c, v) =>
      if (c == uniqueCall) result = v
    }

    while (result == null) {} // TODO: Take a timeout in milliseconds and if this time

    return result
  }

}

case class Call(fnName: String, args: List[Value]) {
  def invoke = Caller.callV(fnName, args)
}

case class UniqueCall(call: Call, callCounter: Int)
