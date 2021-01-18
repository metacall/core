package metacall

sealed trait Value

final case class CharValue(value: Char) extends Value
final case class StringValue(value: String) extends Value
final case class ShortValue(value: Short) extends Value
final case class IntValue(value: Int) extends Value
final case class LongValue(value: Long) extends Value
final case class FloatValue(value: Float) extends Value
final case class DoubleValue(value: Double) extends Value
final case class SizeTValue(value: Long) extends Value
final case class BooleanValue(value: Boolean) extends Value
final case class ArrayValue(value: Vector[Value]) extends Value
final case class MapValue(value: Map[Value, Value]) extends Value
final case object NullValue extends Value
