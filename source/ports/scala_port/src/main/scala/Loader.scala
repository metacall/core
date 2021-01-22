package metacall

import metacall.util._
import cats._, cats.implicits._
import java.nio.file.Paths

/** Loads scripts into MetaCall
  * NOTE: Assumes MetaCall is initialized
  */
object Loader {

  def loadFile[F[_]](runtime: Runtime, filePath: String)(implicit
      FE: ApplicativeError[F, Throwable]
  ): F[Unit] = {
    val absolutePath = Paths.get(filePath).toAbsolutePath().toString()
    val code = Bindings.instance.metacall_load_from_file(
      runtime.toString(),
      Array(absolutePath),
      SizeT(1),
      null
    )

    if (code != 0) FE.raiseError(new Exception("Failed to load script " + absolutePath))
    else FE.unit
  }

  def loadFiles[F[_]](runtime: Runtime, filePaths: Vector[String])(implicit
      FE: ApplicativeError[F, Throwable]
  ) = filePaths.traverse(loadFile[F](runtime, _))

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
