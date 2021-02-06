package metacall

import metacall.util._
import cats._
import java.nio.file.Paths

/** Loads scripts into MetaCall
  * NOTE: Assumes MetaCall is initialized
  * WARNNING: You should always load using `Caller.loadFile` so all files are loaded
  * on MetaCall's thread.
  */
private[metacall] object Loader {

  private[metacall] def loadFilesUnsafe(
      runtime: Runtime,
      filePaths: Vector[String]
  ): Unit = {
    // TODO: Not sure if this is recommendable, we must follow Scala (JVM) import method or let MetaCall handle it
    val absolutePaths =
      filePaths.map(filePath => Paths.get(filePath).toAbsolutePath().toString())

    val code = Bindings.instance.metacall_load_from_file(
      runtime.toString(),
      absolutePaths.toArray,
      SizeT(absolutePaths.size.asInstanceOf[Long]),
      null
    )

    if (code != 0)
      throw new Exception("Failed to load scripts: " + filePaths.mkString(" "))
    else ()
  }

  def loadFileUnsafe(runtime: Runtime, filePath: String) = loadFilesUnsafe(runtime, Vector(filePath))

  def loadFiles[F[_]](runtime: Runtime, filePaths: Vector[String])(implicit
      FE: ApplicativeError[F, Throwable]
  ): F[Unit] =
    try {
      loadFilesUnsafe(runtime, filePaths)
      FE.unit
    } catch {
      case e: Exception => FE.raiseError(e)
    }

  def loadFile[F[_]](runtime: Runtime, filePath: String)(implicit
      FE: ApplicativeError[F, Throwable]
  ) = loadFiles[F](runtime, Vector(filePath))

}

sealed trait Runtime
object Runtime {

  case object Python extends Runtime {
    override def toString() = "py"
  }

  case object Node extends Runtime {
    override def toString() = "node"
  }

}
