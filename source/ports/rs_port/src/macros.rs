#[macro_export]
macro_rules! cstring_enum {
    ($var:ident, $enum:ident) => {{
        let var = $var.to_string();
        match ::std::ffi::CString::new(var.clone()) {
            Ok(str) => Ok(str),
            Err(err) => Err($enum::UnexpectedCStringConversionErr(
                $crate::prelude::MetacallStringConversionError::new(var, err),
            )),
        }
    }};

    ($var:expr, $enum:ident) => {{
        let var = $var;
        cstring_enum!(var, $enum)
    }};
}

#[macro_export]
macro_rules! cstring {
    ($var:ident) => {{
        let var = $var.to_string();
        match ::std::ffi::CString::new(var.clone()) {
            Ok(str) => Ok(str),
            Err(err) => Err($crate::prelude::MetacallStringConversionError::new(
                var, err,
            )),
        }
    }};

    ($var:expr) => {{
        let var = $var;
        cstring!(var)
    }};
}

#[macro_export]
macro_rules! match_object_protocol_all {
    ($any:expr, $var:ident, $action:expr, $($type: ty),*) => {
        match_object_protocol!($any, {
            $( $var: $type => $action, )*
            _ => panic!("The object protocol")
        })
    };
}

#[macro_export]
macro_rules! match_object_protocol {
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
macro_rules! match_object_protocol_ref {
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
macro_rules! match_object_protocol_mut {
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
