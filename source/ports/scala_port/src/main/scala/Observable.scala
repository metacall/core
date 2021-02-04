package metacall

private[metacall] class Observable[T] {
  private var observers = Vector.empty[T => Unit]

  def observe(observer: T => Unit): Unit = {
    observers = observers :+ observer
  }
  def emit(element: T): Unit =
    for {
      observer <- observers
    } observer(element)
}
