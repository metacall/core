// Used for documentation.
#[allow(unused_imports)]
use crate::MetaCallValue;

pub(crate) mod private_macros {
    macro_rules! cstring_enum {
        ($var:ident, $enum:ident) => {{
            let var = $var.to_string();
            match ::std::ffi::CString::new(var.clone()) {
                Ok(str) => Ok(str),
                Err(err) => Err($enum::UnexpectedCStringConversionErr(
                    $crate::MetaCallStringConversionError::new(var, err),
                )),
            }
        }};

        ($var:expr, $enum:ident) => {{
            let var = $var;
            cstring_enum!(var, $enum)
        }};
    }

    macro_rules! cstring {
        ($var:ident) => {{
            let var = $var.to_string();
            match ::std::ffi::CString::new(var.clone()) {
                Ok(str) => Ok(str),
                Err(err) => Err($crate::MetaCallStringConversionError::new(var, err)),
            }
        }};

        ($var:expr) => {{
            let var = $var;
            cstring!(var)
        }};
    }

    pub(crate) use cstring;
    pub(crate) use cstring_enum;
}

#[macro_export]
/// Matches [MetaCallValue](MetaCallValue) trait object. For example: ...
/// ```
/// use metacall::{metacall_untyped_no_arg, match_metacall_value};
///
/// let value = metacall_untyped_no_arg("returns_string_or_number").unwrap();
/// match_metacall_value!(value, {
///     str: String => str,
///     num: i16 => num.to_string(),
///     num: i32 => num.to_string(),
///     num: i64 => num.to_string(),
///     num: f32 => num.to_string(),
///     num: f64 => num.to_string(),
///     _ =>  String::from("Invalid output!")
/// });
/// ```
macro_rules! match_metacall_value {
    ( $any:expr, { $( $var:ident : $type:ty => $arm:expr ),*, _ => $default:expr } ) => (
        $(
            if $any.is::<$type>() {
                let $var = $any.downcast::<$type>().unwrap();
                $arm
            } else
        )*
        {
            $default
        }
    )
}
#[macro_export]
/// Same as [match_metacall_value](match_metacall_value) but gives a reference. For example: ...
/// ```
/// use metacall::{metacall_untyped_no_arg, match_metacall_value_ref};
///
/// let value = metacall_untyped_no_arg("returns_string_or_number").unwrap();
/// match_metacall_value_ref!(value, {
///     str: String => str.to_owned(),
///     num: i16 => num.to_string(),
///     num: i32 => num.to_string(),
///     num: i64 => num.to_string(),
///     num: f32 => num.to_string(),
///     num: f64 => num.to_string(),
///     _ =>  String::from("Invalid output!")
/// });
/// ```
macro_rules! match_metacall_value_ref {
    ( $any:expr, { $( $var:ident : $type:ty => $arm:expr ),*, _ => $default:expr } ) => (
        $(
            if $any.is::<$type>() {
                let $var = $any.downcast_ref::<$type>().unwrap();
                $arm
            } else
        )*
        {
            $default
        }
    )
}
#[macro_export]
/// Same as [match_metacall_value](match_metacall_value) but gives a mutable reference. For example: ...
/// ```
/// use metacall::{metacall_untyped_no_arg, match_metacall_value_mut};
///
/// let mut value = metacall_untyped_no_arg("returns_string_or_number").unwrap();
/// match_metacall_value_mut!(value, {
///     str: String => str.to_owned(),
///     num: i16 => num.to_string(),
///     num: i32 => num.to_string(),
///     num: i64 => num.to_string(),
///     num: f32 => num.to_string(),
///     num: f64 => num.to_string(),
///     _ =>  String::from("Invalid output!")
/// });
/// ```
macro_rules! match_metacall_value_mut {
    ( $any:expr, { $( $var:ident : $type:ty => $arm:expr ),*, _ => $default:expr } ) => (
        $(
            if $any.is::<$type>() {
                let $var = $any.downcast_mut::<$type>().unwrap();
                $arm
            } else
        )*
        {
            $default
        }
    )
}
