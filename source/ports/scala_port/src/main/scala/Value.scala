package metacall

sealed trait Value

trait Args[A] {
  def from(product: A): List[Value]
}

sealed abstract class NumericValue[N](implicit n: Numeric[N]) extends Value {
  val value: N

  def int = IntValue(n.toInt(value))
  def long = LongValue(n.toLong(value))
  def float = FloatValue(n.toFloat(value))
  def double = DoubleValue(n.toDouble(value))
}

final case class CharValue(value: Char) extends Value
final case class StringValue(value: String) extends Value
final case class ShortValue(value: Short) extends NumericValue[Short]
final case class IntValue(value: Int) extends NumericValue[Int]
final case class LongValue(value: Long) extends NumericValue[Long]
final case class FloatValue(value: Float) extends NumericValue[Float]
final case class DoubleValue(value: Double) extends NumericValue[Double]
final case class BooleanValue(value: Boolean) extends Value
final case class ArrayValue(value: Vector[Value]) extends Value
final case class MapValue(value: Map[Value, Value]) extends Value
final case class FunctionValue(value: List[Value] => Value) extends Value
final case object NullValue extends Value
final case object InvalidValue extends Value
