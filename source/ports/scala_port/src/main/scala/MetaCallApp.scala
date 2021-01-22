package metacall

import cats.implicits._, cats.effect._

trait MetaCallApp extends IOApp {

  def run(args: List[String]): IO[ExitCode]

  val imports: Map[Runtime, Vector[String]]

  final override def main(args: Array[String]): Unit = {
    Bindings.instance.metacall_initialize()

    imports.toVector
      .traverse { case (runtime, paths) =>
        Loader.loadFiles[IO](runtime, paths)
      }
      .unsafeRunSync()

    super.main(args)
    Bindings.instance.metacall_destroy()
  }

  final def call(fnName: String, args: Vector[Value]) = Caller.call[IO](fnName, args)

}
