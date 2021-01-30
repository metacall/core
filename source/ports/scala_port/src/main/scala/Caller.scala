package metacall

import metacall.util._

object Caller {

  /** Calls a loaded function
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  def callV(fnName: String, args: List[Value]): Value = {
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
  def call[A](fnName: String, args: A)(implicit AA: Args[A]): Value =
    callV(fnName, AA.from(args))

}
