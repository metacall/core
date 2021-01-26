package metacall

import metacall.util._
import cats._, cats.implicits._, cats.effect._

object Caller {

  def call[F[_]](fnName: String, args: Vector[Value])(implicit
      FE: MonadError[F, Throwable],
      FD: Defer[F],
      BF: BracketThrow[F]
  ): F[Value] = {
    val argPtrArray = args.traverse(Ptr.fromValue[F]).map(_.map(_.ptr).toArray)

    val retPtr = argPtrArray
      .flatMap { args =>
        Ptr.fromPrimitive[F] {
          Bindings.instance.metacallv_s(fnName, args, SizeT(args.length.toLong))
        }
      }
      .map(Ptr.toValue)

    retPtr.use(FE.pure)
  }

}
