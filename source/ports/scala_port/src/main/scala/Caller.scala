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
    argPtrArray
      .evalMap { args =>
        val retPtr = Ptr.fromPrimitive[F] {
          Bindings.instance.metacallv_s(fnName, args, SizeT(args.length.toLong))
        }

        retPtr.flatMap { retPtr =>
          Ptr.toValue[F](retPtr) <*
            FD.defer(FE.pure(Bindings.instance.metacall_value_destroy(retPtr.ptr)))
        }
      }
      .use(FE.pure)
  }

}
