package metacall

import metacall.util._
import scala.concurrent.Future
import scala.concurrent.ExecutionContext

import com.sun.jna._, ptr.PointerByReference
import java.util.concurrent.{ConcurrentLinkedQueue, ConcurrentHashMap}
import java.util.concurrent.locks.{ReentrantLock}
import java.util.concurrent.atomic.{AtomicBoolean, AtomicInteger}
import scala.util._

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
  *  Caller.start()
  *  Caller.loadFile(Runtime.Python, "./src/test/scala/scripts/main.py")
  *  val ret = Caller.call("big_fn", (1, "hello", 2.2))
  *  assert(ret == DoubleValue(8.2))
  *  ```
  */
object Caller {

  private final case class Call(
      id: Int,
      namespace: Option[String],
      fnName: String,
      args: List[Value]
  )

  private final case class LoadCommand(
      id: Int,
      namespace: Option[String],
      runtime: Runtime,
      filePaths: Vector[String]
  )

  private def callLoop() = {
    if (!Bindings.runningInMetacall)
      Bindings.instance.metacall_initialize()

    while (!closed.get) {
      if (!scriptsQueue.isEmpty()) {
        val LoadCommand(id, namespace, runtime, paths) =
          scriptsQueue.poll()
        try {
          scriptsLock.lock()
          val handleRef = namespace.map(_ => new PointerByReference())
          val loadResult = Loader.loadFilesUnsafe(runtime, paths, handleRef)

          loadResult match {
            case Success(()) => {
              handleRef zip namespace match {
                case Some((handleRef, namespace)) =>
                  namespaceHandles.put(
                    namespace,
                    handleRef
                  )
                case None => ()
              }
              scriptLoadResults.put(id, Success(()))
            }
            case Failure(e) => scriptLoadResults.put(id, Failure(e))
          }
        } catch {
          case e: Throwable => scriptLoadResults.put(id, Failure(e))
        } finally {
          scriptsReady.signal()
          scriptsLock.unlock()
        }
      } else if (!callQueue.isEmpty()) {
        val Call(id, namespace, fnName, args) = callQueue.poll()
        val result = callUnsafe(namespace, fnName, args)
        callResultMap.put(id, result)
      }
    }

    if (!Bindings.runningInMetacall)
      Bindings.instance.metacall_destroy()
  }

  private val scriptsLock = new ReentrantLock()
  private val scriptsReady = scriptsLock.newCondition()
  private val closed = new AtomicBoolean(false)
  private val callQueue = new ConcurrentLinkedQueue[Call]()
  private val callResultMap = new ConcurrentHashMap[Int, Try[Value]]()
  private val callCounter = new AtomicInteger(0)
  private val scriptsQueue = new ConcurrentLinkedQueue[LoadCommand]()
  private val scriptLoadResults = new ConcurrentHashMap[Int, Try[Unit]]()
  private val scriptLoadCounter = new AtomicInteger(0)
  private val namespaceHandles =
    new ConcurrentHashMap[String, PointerByReference]()

  def loadFiles(
      runtime: Runtime,
      filePaths: Vector[String],
      namespace: Option[String] = None
  ): Try[Unit] = {
    if (closed.get()) {
      val scriptsStr =
        if (filePaths.length == 1) "script " + filePaths.head
        else "scripts " + filePaths.mkString(", ")
      return Failure {
        new Exception(
          s"Trying to load scripts $scriptsStr while the caller is closed"
        )
      }
    }

    val loadId = scriptLoadCounter.getAndIncrement()

    if (loadId == Int.MaxValue - 1)
      scriptLoadCounter.set(0)

    scriptsQueue.add(LoadCommand(loadId, namespace, runtime, filePaths))

    scriptsLock.lock()

    while (!scriptsQueue.isEmpty())
      scriptsReady.await()

    scriptsLock.unlock()

    scriptLoadResults.get(loadId)
  }

  def loadFile(
      runtime: Runtime,
      filePath: String,
      namespace: Option[String] = None
  ): Try[Unit] = loadFiles(runtime, Vector(filePath), namespace)

  def loadFile(runtime: Runtime, filePath: String, namespace: String): Try[Unit] =
    loadFile(runtime, filePath, Some(namespace))

  def loadFile(runtime: Runtime, filePath: String): Try[Unit] =
    loadFile(runtime, filePath, None)

  /** Starts the MetaCall instance.
    * WARNING: Should only be called once.
    * @param ec The `ExecutionContext` in which all (non-blocking) function calls are executed.
    */
  def start(ec: ExecutionContext): Unit =
    ec.execute(() => concurrent.blocking(callLoop()))

  /** Destroys MetaCall.
    * WARNING: Should only be called once during the life of the application.
    * Destroying and restarting may result in unexpected runtime failure.
    */
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
  ): Try[Value] = {
    val argPtrArray = args.map(Ptr.fromValueUnsafe(_).ptr).toArray

    val retPointer =
      namespace match {
        case Some(value) => {
          val namespaceHandle = namespaceHandles.get(value)

          if (namespaceHandle == null)
            Failure {
              new Exception(
                s"Namespace `$value` does not contain any functions (no scripts were loaded in it)"
              )
            }
          else
            Success {
              Bindings.instance.metacallhv_s(
                namespaceHandle.getValue(),
                fnName,
                argPtrArray,
                SizeT(argPtrArray.length.toLong)
              )
            }
        }
        case None =>
          Success {
            Bindings.instance.metacallv_s(
              fnName,
              argPtrArray,
              SizeT(argPtrArray.length.toLong)
            )
          }
      }

    val retValue = retPointer.map(retp => Ptr.toValue(Ptr.fromPrimitiveUnsafe(retp)))

    retPointer.foreach(Bindings.instance.metacall_value_destroy)
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
    Future {
      concurrent.blocking {
        blocking.callV(fnName, args, namespace).get
      }
    }

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
    ): Try[Value] = {
      val callId = callCounter.getAndIncrement()

      if (callId == Int.MaxValue - 1)
        callCounter.set(0)

      callQueue.add(Call(callId, namespace, fnName, args))

      var result: Try[Value] = null

      while (result == null)
        result = callResultMap.get(callId)

      callResultMap.remove(callId)

      result
    }

    /** Calls a loaded function
      * @param fnName The name of the function to call
      * @param args A product (tuple, case class, single value) to be passed as arguments to the function
      * @param namespace The script/module file where the function is defined
      * @return The function's return value
      */
    def call[A](
        fnName: String,
        args: A,
        namespace: Option[String] = None
    )(implicit AA: Args[A]): Try[Value] =
      blocking.callV(fnName, AA.from(args), namespace)

  }

}
