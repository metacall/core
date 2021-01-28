package metacall

import metacall.util._

object Caller {

  def call(fnName: String, args: Vector[Value]): Value = {
    val argPtrArray = args.map(Ptr.fromValueUnsafe(_).ptr).toArray

    val retPointer =
      Bindings.instance.metacallv_s(fnName, argPtrArray, SizeT(argPtrArray.length.toLong))

    val retValue = Ptr.toValue(Ptr.fromPrimitiveUnsafe(retPointer))

    Bindings.instance.metacall_value_destroy(retPointer)
    argPtrArray.foreach(Bindings.instance.metacall_value_destroy)

    retValue
  }

}
