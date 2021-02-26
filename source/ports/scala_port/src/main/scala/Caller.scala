package metacall

import metacall.util._
import scala.concurrent.{Future, Promise}
import scala.concurrent.ExecutionContext

import com.sun.jna._, ptr.PointerByReference
import java.util.concurrent.{ConcurrentLinkedQueue, ConcurrentHashMap}
import scala.util._

/** `Caller` creates a new thread on which:
  *   - a MetaCall instance is initialized (`Caller.start`)
  *   - Scripts are loaded (`Caller.loadFile`)
  *   - MetaCall functions are invoked (`Caller.call`)
  *
  *  You must always call `Caller.destroy` after you're done with it. This destroys
  *  the MetaCall instance.
  */
object Caller {

  private final case class Call(
      namespace: Option[String],
      fnName: String,
      args: List[Value],
      resultPromise: Promise[Value]
  )

  private final case class Load(
      namespace: Option[String],
      runtime: Runtime,
      filePaths: Vector[String],
      resultPromise: Promise[Unit]
  )

  private final case class Namespace(
      handle: PointerByReference,
      definitions: Map[String, FunctionMetadata]
  )

  final case class FunctionMetadata(isAsync: Boolean)

  private def callLoop(disableLogging: Boolean) = {
    if (!Bindings.runningInMetacall) {
      if (disableLogging)
        Bindings.instance.metacall_log_null()

      Bindings.instance.metacall_initialize()
    }

    while (!closed) {
      if (!scriptsQueue.isEmpty()) {
        val Load(namespace, runtime, paths, resPromise) = scriptsQueue.poll()
        val handleRef = namespace.map(_ => new PointerByReference())
        val loadResult = Loader.loadFilesUnsafe(runtime, paths, handleRef)

        loadResult match {
          case Success(()) => {
            handleRef zip namespace match {
              case Some((handleRef, namespaceName)) => {
                val mapPtr = Ptr.fromPrimitiveUnsafe {
                  Bindings.instance.metacall_handle_export(handleRef.getValue())
                }

                val definitions = Try(mapPtr)
                  .flatMap {
                    case p: MapPtr => Success(instances.mapGet.primitive(p))
                    case _ =>
                      Failure {
                        new Exception(
                          s"Unable to get metadata from namespace `$namespaceName` (metadata must be a map)"
                        )
                      }
                  }
                  .map { pairs =>
                    println(
                      s"============== Pairs length of ${namespaceName}: ${pairs.length} ==================="
                    )
                    pairs.map { case (fnNamePointer, fnPointer) =>
                      Bindings.instance.metacall_value_to_string(fnNamePointer) -> {
                        val isAsync =
                          Bindings.instance.metacall_function_async(fnPointer) == 1

                        FunctionMetadata(isAsync)
                      }
                    }.toMap
                  }

                Bindings.instance.metacall_value_destroy(mapPtr.ptr)

                definitions match {
                  case Success(defs) => {
                    namespaces.put(
                      namespaceName,
                      Namespace(handleRef, defs)
                    )

                    resPromise.success(())
                  }
                  case Failure(e) => resPromise.failure(e)
                }
              }
              case None => resPromise.success(())
            }
          }
          case Failure(e) => resPromise.failure(e)
        }
      } else if (!callQueue.isEmpty()) {
        val Call(namespace, fnName, args, resultPromise) = callQueue.poll()
        resultPromise.tryComplete(callUnsafe(namespace, fnName, args))
      }
    }

    if (!Bindings.runningInMetacall)
      Bindings.instance.metacall_destroy()
  }

  private var closed = true
  private var startedOnce = false
  private val callQueue = new ConcurrentLinkedQueue[Call]()
  private val scriptsQueue = new ConcurrentLinkedQueue[Load]()
  private val namespaces = new ConcurrentHashMap[String, Namespace]()

  def loadFiles(
      runtime: Runtime,
      filePaths: Vector[String],
      namespace: Option[String] = None
  ): Future[Unit] = {
    if (closed) {
      val scriptsStr =
        if (filePaths.length == 1) "script " + filePaths.head
        else "scripts " + filePaths.mkString(", ")
      return Future.failed {
        new Exception(
          s"Trying to load scripts $scriptsStr while the caller is closed"
        )
      }
    }

    val resPromise = Promise[Unit]()

    scriptsQueue.add(Load(namespace, runtime, filePaths, resPromise))

    resPromise.future
  }

  def loadFile(
      runtime: Runtime,
      filePath: String,
      namespace: Option[String] = None
  ): Future[Unit] = loadFiles(runtime, Vector(filePath), namespace)

  def loadFile(runtime: Runtime, filePath: String, namespace: String): Future[Unit] =
    loadFile(runtime, filePath, Some(namespace))

  def loadFile(runtime: Runtime, filePath: String): Future[Unit] =
    loadFile(runtime, filePath, None)

  /** @return functions defined in `namespace` */
  def definitions(namespace: String): Option[Map[String, FunctionMetadata]] =
    Option(namespaces.get(namespace)).map(_.definitions)

  /** Starts the MetaCall instance.
    * WARNING: Should only be called once.
    * @param ec The `ExecutionContext` in which all (non-blocking) function calls are executed.
    */
  def start(ec: ExecutionContext, disableLogging: Boolean = true): Try[Unit] =
    if (startedOnce) Failure(new Exception("Caller has already been started once before"))
    else
      Success {
        synchronized {
          startedOnce = true
          closed = false
        }

        ec.execute(() => concurrent.blocking(callLoop(disableLogging)))
      }

  /** Destroys MetaCall.
    * WARNING: Should only be called once during the life of the application.
    * Destroying and restarting may result in unexpected runtime failure.
    */
  def destroy(): Unit = synchronized { closed = true }

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
        case Some(namespaceName) => {
          val namespace = namespaces.get(namespaceName)

          if (namespace == null)
            Failure {
              new Exception(
                s"Namespace `$namespaceName` is not defined (no scripts were loaded in it)"
              )
            }
          else if (!namespace.definitions.contains(fnName))
            Failure {
              new Exception(
                s"Function `$fnName` is not defined in `$namespaceName`"
              )
            }
          else
            Success {
              Bindings.instance.metacallhv_s(
                namespace.handle.getValue(),
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

  /** Calls a loaded function with a list of `metacall.Value` arguments.
    * @param fnName The name of the function to call
    * @param args A list of `Value`s to be passed as arguments to the function
    * @param namespace The script/module file where the function is defined
    * @return The function's return value, or `InvalidValue` in case of an error
    */
  def callV(
      fnName: String,
      args: List[Value],
      namespace: Option[String] = None
  ): Future[Value] = {
    val result = Promise[Value]()

    callQueue.add(Call(namespace, fnName, args, result))

    result.future
  }

  /** Calls a loaded function with a product value as the arguments.
    * @param fnName The name of the function to call
    * @param args A product (tuple, case class, single value) to be passed as arguments to the function
    * @param namespace The script/module file where the function is defined
    * @return The function's return value
    */
  def call[A](fnName: String, args: A, namespace: Option[String] = None)(implicit
      AA: Args[A]
  ): Future[Value] =
    callV(fnName, AA.from(args), namespace)

  /** Calls a loaded function with a product value as the arguments.
    * @param namespace The namespace where the function is loaded
    * @param fnName The name of the function to call
    * @param args A product (tuple, case class, single value) to be passed as arguments to the function
    * @return The function's return value
    */
  def call[A](namespace: String, fnName: String, args: A)(implicit
      AA: Args[A]
  ): Future[Value] = call[A](fnName, args, Some(namespace))

}
