package metacall

import metacall.util._
import java.nio.file.Paths
import com.sun.jna._, ptr.PointerByReference

/** Loads scripts into MetaCall
  * NOTE: Assumes MetaCall is initialized
  * WARNNING: You should always load using `Caller.loadFile` so all files are loaded
  * on MetaCall's thread.
  */
private[metacall] object Loader {

  private[metacall] def loadFilesUnsafe(
      runtime: Runtime,
      filePaths: Vector[String],
      handleRef: Option[PointerByReference]
  ): Unit = {
    val absolutePaths =
      filePaths.map(filePath => Paths.get(filePath).toAbsolutePath().toString())

    val code = Bindings.instance.metacall_load_from_file(
      runtime.toString(),
      absolutePaths.toArray,
      SizeT(absolutePaths.size.asInstanceOf[Long]),
      handleRef match {
        case Some(ref) => ref
        case None      => null
      }
    )

    if (code != 0)
      throw new Exception("Failed to load scripts: " + filePaths.mkString(" "))
  }
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
