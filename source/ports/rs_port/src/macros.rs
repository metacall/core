#[macro_export]
macro_rules! cstring_enum {
    ($var:ident, $enum:ident) => {{
        let var = $var.to_string();
        match ::std::ffi::CString::new(var.clone()) {
            Ok(str) => Ok(str),
            Err(err) => Err($enum::UnexpectedCStringConversionErr(
                crate::prelude::MetacallStringConversionError::new(var, err),
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
            Err(err) => Err(crate::prelude::MetacallStringConversionError::new(var, err)),
        }
    }};

    ($var:expr) => {{
        let var = $var;
        cstring!(var)
    }};
}

#[macro_export]
macro_rules! any_unwrap {
    ($var:ident, $enum:ident) => {{
        if let crate::prelude::Any::$enum(x) = $var {
            x
        } else {
            panic!(
                "Failed to parse `{:#?}` as `{:#?}`!",
                $var,
                stringify!($enum)
            );
        }
    }};

    ($var:expr, $enum:ident) => {{
        let var = $var;
        any_unwrap!(var, $enum)
    }};
}
