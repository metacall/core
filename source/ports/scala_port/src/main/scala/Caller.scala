package metacall

import metacall.util._
import scala.concurrent.Future
import scala.concurrent.ExecutionContext

import com.sun.jna._, ptr.PointerByReference
import java.util.concurrent.{ConcurrentLinkedQueue, ConcurrentHashMap}
import java.util.concurrent.locks.{ReentrantLock}
import java.util.concurrent.atomic.{AtomicBoolean, AtomicInteger}

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

  private case class LoadCommand(
      namespace: Option[String],
      runtime: Runtime,
      filePaths: Vector[String]
  )

  private val runningInMetacall = System.getProperty("metacall.polyglot.name") == "core"

  private def callLoop() = {
    if (!runningInMetacall)
      Bindings.instance.metacall_initialize()

    while (!closed.get) {
      if (!scriptsQueue.isEmpty()) {
        try {
          scriptsLock.lock()
          val LoadCommand(namespace, runtime, paths) = scriptsQueue.poll()
          val handleRef = namespace.map(_ => new PointerByReference())
          Loader.loadFilesUnsafe(runtime, paths, handleRef)
          handleRef.zip(namespace) match {
            case Some((handleRef, namespace)) =>
              namespaceHandles.put(
                namespace,
                handleRef
              )
            case None => ()
          }
          // TODO: We may need to set up the result or the error in a monad
        } catch {
          // TODO: We may need to set up the result or the error in a monad
          case e: Throwable => Console.err.println(e)
        } finally {
          scriptsReady.signal()
          scriptsLock.unlock()
        }
      } else if (!callQueue.isEmpty()) {
        try {
          val UniqueCall(Call(namespace, fnName, args), id) = callQueue.poll()
          val result = callUnsafe(namespace, fnName, args)
          callResultMap.put(id, result)
        } catch {
          case e: Throwable => Console.err.println(e)
        }
      }
    }

    if (!runningInMetacall)
      Bindings.instance.metacall_destroy()
  }

  private val scriptsLock = new ReentrantLock()
  private val scriptsReady = scriptsLock.newCondition()
  private val closed = new AtomicBoolean(false)
  private val callQueue = new ConcurrentLinkedQueue[UniqueCall]()
  private val callResultMap = new ConcurrentHashMap[Int, Value]()
  private val callCounter = new AtomicInteger(0)
  private val scriptsQueue = new ConcurrentLinkedQueue[LoadCommand]()
  private val namespaceHandles =
    new ConcurrentHashMap[String, PointerByReference]()

  def loadFiles(
      runtime: Runtime,
      filePaths: Vector[String],
      namespace: Option[String] = None
  ): Unit = {
    if (closed.get()) {
      val scriptsStr =
        if (filePaths.length == 1) "script " + filePaths.head
        else "scripts " + filePaths.mkString(", ")
      throw new Exception(
        s"Trying to load scripts $scriptsStr while the caller is closed"
      )
    }

    scriptsQueue.add(LoadCommand(namespace, runtime, filePaths))

    scriptsLock.lock()

    while (!scriptsQueue.isEmpty())
      scriptsReady.await()

    scriptsLock.unlock()
  }

  def loadFile(
      runtime: Runtime,
      filePath: String,
      namespace: Option[String] = None
  ): Unit = loadFiles(runtime, Vector(filePath), namespace)

  def loadFile(runtime: Runtime, filePath: String, namespace: String): Unit =
    loadFile(runtime, filePath, Some(namespace))

  def loadFile(runtime: Runtime, filePath: String): Unit =
    loadFile(runtime, filePath, None)

  def start(): Unit = {
    new Thread(() => callLoop()).start()
  }

  def destroy(): Unit = closed.set(true)

  /** Calls a loaded function.
    * WARNING: Should only be used from within the caller thread.
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @param namespace The script/module file where the function is defined
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  private def callUnsafe(
      namespace: Option[String],
      fnName: String,
      args: List[Value]
  ): Value = {
    val argPtrArray = args.map(Ptr.fromValueUnsafe(_).ptr).toArray

    val retPointer =
      namespace match {
        case Some(value) => {
          val namespaceHandle = namespaceHandles.get(value)

          if (namespaceHandle == null)
            throw new Exception(
              s"Namespace `$value` does not contain any functions (no scripts were loaded in it)"
            )

          Bindings.instance.metacallhv_s(
            namespaceHandle.getValue(),
            fnName,
            argPtrArray,
            SizeT(argPtrArray.length.toLong)
          )
        }
        case None => {
          Bindings.instance.metacallv_s(
            fnName,
            argPtrArray,
            SizeT(argPtrArray.length.toLong)
          )
        }
      }

    val retValue = Ptr.toValue(Ptr.fromPrimitiveUnsafe(retPointer))

    Bindings.instance.metacall_value_destroy(retPointer)
    argPtrArray.foreach(Bindings.instance.metacall_value_destroy)

    retValue
  }

  /** Calls a loaded function.
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @param namespace The script/module file where the function is defined
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  def callV(fnName: String, args: List[Value], namespace: Option[String] = None)(implicit
      ec: ExecutionContext
  ): Future[Value] =
    Future(blocking.callV(fnName, args, namespace))

  def call[A](fnName: String, args: A, namespace: Option[String] = None)(implicit
      AA: Args[A],
      ec: ExecutionContext
  ): Future[Value] =
    callV(fnName, AA.from(args), namespace)

  def call[A](namespace: String, fnName: String, args: A)(implicit
      AA: Args[A],
      ec: ExecutionContext
  ): Future[Value] = call[A](fnName, args, Some(namespace))

  /** Blocking versions of the methods on [[Caller]]. Do not use them if you don't *need* to. */
  object blocking {

    /** Calls a loaded function.
      * @param fnName The name of the function to call
      * @param args A list of `Value`s to be passed as arguments to the function
      * @param namespace The script/module file where the function is defined
      * @return The function's return value, or `InvalidValue` in case of an error
      */
    def callV(
        fnName: String,
        args: List[Value],
        namespace: Option[String] = None
    ): Value = {
      val call = Call(namespace, fnName, args)

      val callId = callCounter.getAndIncrement()

      if (callId == Int.MaxValue - 1)
        callCounter.set(0)

      val uniqueCall = UniqueCall(call, callId)

      callQueue.add(uniqueCall)

      var result: Value = null

      while (result == null)
        result = callResultMap.get(callId)

      callResultMap.remove(callId)

      result
    }

    /** Calls a loaded function
      * @param fnName The name of the function to call
      * @param args A product (tuple, case class, single value) to be passed as arguments to the function
      * @param namespace The script/module file where the function is defined
      * @return The function's return value, or `InvalidValue` in case of an error
      */
    def call[A](
        fnName: String,
        args: A,
        namespace: Option[String] = None
    )(implicit AA: Args[A]): Value =
      blocking.callV(fnName, AA.from(args), namespace)

  }

}
