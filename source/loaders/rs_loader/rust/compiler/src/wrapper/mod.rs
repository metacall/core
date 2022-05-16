mod array;
mod map;
mod number;
mod string;

use crate::Function;

use super::{
    config::Input, source_map::FileName::Custom, CompilerCallbacks, FunctionParameter,
    FunctionType, Source,
};
use std::fmt;
use std::fs::File;
use std::io::Write;

trait Wrapper {
    fn as_arg(&self) -> String;
    fn transform(&self, args_ptr: &str) -> String;
    fn cleanup(&self) -> String;
    fn arg_name(&self) -> String;
    fn var_name(&self) -> String;
    fn get_args_type(&self) -> FunctionParameter;
    fn get_ret_type(&self) -> FunctionParameter;
    fn handle_ret(&self, ret_name: &str) -> String;
}

fn value_to_type(ty: &FunctionType) -> String {
    match ty {
        FunctionType::I16 | FunctionType::U16 => "metacall_value_to_short".to_string(),
        FunctionType::I32 | FunctionType::U32 => "metacall_value_to_int".to_string(),
        FunctionType::I64 | FunctionType::U64 => "metacall_value_to_long".to_string(),
        FunctionType::Usize => "metacall_value_to_long".to_string(),
        FunctionType::Bool => "metacall_value_to_bool".to_string(),
        FunctionType::Char => "metacall_value_to_char".to_string(),
        FunctionType::F32 => "metacall_value_to_float".to_string(),
        FunctionType::F64 => "metacall_value_to_double".to_string(),
        _ => todo!(),
    }
}

fn value_to_rust_type(ty: &FunctionType) -> String {
    match ty {
        FunctionType::I16 => "i16".to_string(),
        FunctionType::I32 => "i32".to_string(),
        FunctionType::I64 => "i64".to_string(),
        FunctionType::U16 => "u16".to_string(),
        FunctionType::U32 => "u32".to_string(),
        FunctionType::U64 => "u64".to_string(),
        FunctionType::Usize => "usize".to_string(),
        FunctionType::Bool => "bool".to_string(),
        FunctionType::Char => "char".to_string(),
        FunctionType::F32 => "f32".to_string(),
        FunctionType::F64 => "f64".to_string(),
        _ => todo!(),
    }
}

fn value_create_type(ty: &FunctionParameter, ret_name: &str) -> String {
    match ty.ty {
        FunctionType::I16 | FunctionType::U16 => {
            format!("metacall_value_create_short({ret_name}.try_into().unwrap())")
        }
        FunctionType::I32 | FunctionType::U32 => {
            format!("metacall_value_create_int({ret_name}.try_into().unwrap())")
        }
        FunctionType::I64 | FunctionType::U64 | FunctionType::Usize => {
            format!("metacall_value_create_long({ret_name}.try_into().unwrap())")
        }
        FunctionType::Bool => format!("metacall_value_create_bool({ret_name}.try_into().unwrap())"),
        FunctionType::Char => format!("metacall_value_create_char({ret_name}.try_into().unwrap())"),
        FunctionType::F32 => format!("metacall_value_create_float({ret_name}.try_into().unwrap())"),
        FunctionType::F64 => {
            format!("metacall_value_create_double({ret_name}.try_into().unwrap())")
        }
        FunctionType::Array => {
            let genreic_typ = &ty.generic[0];
            format!(
                "let ret_vec = {ret_name}.into_iter().map(|val| {{ {} }}).collect::<Vec<*mut c_void>>();
            metacall_value_create_array(ret_vec.as_ptr(), ret_vec.len())",
                value_create_type(genreic_typ, "val"),
            )
        }
        FunctionType::Map => {
            let key_typ = &ty.generic[0];
            let val_typ = &ty.generic[1];
            format!(
                "let size = {ret_name}.len();
                let ret_map = {ret_name}.into_iter()
            .map(|(key, val)| {{
                let pair = vec![{}, {}];
                metacall_value_create_array(pair.as_ptr(), pair.len())
            }})
            .collect::<Vec<*mut c_void>>();
            metacall_value_create_map(ret_map.as_ptr(), size)",
                value_create_type(key_typ, "key"),
                value_create_type(val_typ, "val")
            )
        }
        FunctionType::String => {
            format!(
                "metacall_value_create_string({ret_name}.as_ptr() as *const i8, {ret_name}.len())",
            )
        }
        _ => todo!(),
    }
}

impl fmt::Debug for dyn Wrapper {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(format!("{:?}", self).as_str())?;
        Ok(())
    }
}

#[derive(Default, Debug)]
struct WrapperFunction {
    name: String,
    args: Vec<Box<dyn Wrapper>>,
    ret: Option<Box<dyn Wrapper>>,
}

fn function_to_wrapper(idx: usize, typ: &FunctionParameter) -> Box<dyn Wrapper> {
    match typ.ty {
        FunctionType::I16
        | FunctionType::I32
        | FunctionType::I64
        | FunctionType::U16
        | FunctionType::U32
        | FunctionType::U64
        | FunctionType::Usize
        | FunctionType::F32
        | FunctionType::F64 => Box::new(number::Number::new(idx, typ.clone())),
        FunctionType::Array => Box::new(array::Vec::new(idx, typ.clone())),
        FunctionType::Map => Box::new(map::Map::new(idx, typ.clone())),
        FunctionType::String => Box::new(string::MString::new(idx, typ.clone())),
        // FunctionType::Null => Box::new(null::Null{}),
        _ => todo!(),
    }
}

impl WrapperFunction {
    fn new(func: &Function) -> Self {
        let mut result = WrapperFunction {
            name: func.name.clone(),
            args: vec![],
            ret: None,
        };
        if let Some(ret) = &func.ret {
            result.ret = Some(function_to_wrapper(0, ret));
        }
        for (idx, arg) in func.args.iter().enumerate() {
            result.args.push(function_to_wrapper(idx, arg));
        }
        result
    }

    fn generate(&self) -> String {
        let mut wrapper_string = String::new();
        wrapper_string.push_str(
            format!(
                "#[no_mangle]
pub unsafe fn metacall_{}(args_p: *mut *mut c_void, size: usize) -> *mut c_void {{
",
                self.name
            )
            .as_str(),
        );
        wrapper_string.push_str("\tlet args_ptr = std::slice::from_raw_parts(args_p, size);\n");

        // transform
        for arg in self.args.iter() {
            wrapper_string.push_str(format!("\t{}", arg.transform("args_ptr")).as_str());
        }

        // call real_func
        wrapper_string.push_str(
            format!(
                "\tlet metacall_res = {}({});\n",
                self.name,
                self.args
                    .iter()
                    .map(|arg| arg.var_name())
                    .collect::<Vec<String>>()
                    .join(", ")
            )
            .as_str(),
        );

        // cleanup
        for arg in self.args.iter() {
            wrapper_string.push_str(format!("\t{}", arg.cleanup()).as_str());
        }

        if let Some(ret) = &self.ret {
            wrapper_string
                .push_str(format!("\t{} \n}}\n", ret.handle_ret("metacall_res")).as_str());
        } else {
            wrapper_string.push_str("\t0 as *mut c_void \n}\n");
        }

        wrapper_string
    }
}

pub fn generate_wrapper(callbacks: CompilerCallbacks) -> std::io::Result<CompilerCallbacks> {
    let mut wrapped_functions: Vec<Function> = vec![];
    let mut content = String::new();
    content.push_str(
        "
use std::{
    ffi::{c_void, CString, CStr},
    os::raw::{c_char, c_double, c_float, c_int, c_long, c_short},
};
extern \"C\" {
    fn value_type_count(v: *mut c_void) -> c_int;
    fn metacall_value_id(v: *mut c_void) -> c_int;
    fn metacall_value_to_int(v: *mut c_void) -> c_int;
    fn metacall_value_to_bool(v: *mut c_void) -> c_int;
    fn metacall_value_to_char(v: *mut c_void) -> c_char;
    fn metacall_value_to_long(v: *mut c_void) -> c_long;
    fn metacall_value_to_short(v: *mut c_void) -> c_short;
    fn metacall_value_to_float(v: *mut c_void) -> c_float;
    fn metacall_value_to_double(v: *mut c_void) -> c_double;
    fn metacall_value_to_array(v: *mut c_void) -> *mut *mut c_void;
    fn metacall_value_to_map(v: *mut c_void) -> *mut *mut c_void;
    fn metacall_value_to_ptr(v: *mut c_void) -> *mut c_void;
    fn metacall_value_to_string(v: *mut c_void) -> *mut c_char;
    fn metacall_function(cfn: *const c_char) -> *mut c_void;
    fn metacall_value_create_int(i: c_int) -> *mut c_void;
    fn metacall_value_create_bool(b: c_int) -> *mut c_void;
    fn metacall_value_create_long(l: c_long) -> *mut c_void;
    fn metacall_value_create_char(st: c_char) -> *mut c_void;
    fn metacall_value_create_short(s: c_short) -> *mut c_void;
    fn metacall_value_create_float(f: c_float) -> *mut c_void;
    fn metacall_value_create_double(d: c_double) -> *mut c_void;
    fn metacall_value_create_string(st: *const c_char, ln: usize) -> *mut c_void;
    fn metacall_value_create_array(values: *const *mut c_void, size: usize) -> *mut c_void;
    fn metacall_value_create_map(tuples: *const *mut c_void, size: usize) -> *mut c_void;
}
    ",
    );
    for func in callbacks.functions.iter() {
        let wrapper_func = WrapperFunction::new(func);
        let wrapper = wrapper_func.generate();
        content.push_str(wrapper.as_str());

        let mut new_function = Function {
            name: format!("metacall_{}", wrapper_func.name),
            ret: None,
            args: vec![],
        };
        if let Some(ret) = wrapper_func.ret {
            new_function.ret = Some(ret.get_ret_type());
        }
        for arg in wrapper_func.args.iter() {
            new_function.args.push(arg.get_args_type());
        }
        wrapped_functions.push(new_function);
    }
    match callbacks.source.input.0 {
        Input::File(input_path) => {
            // generate wrappers to a file source_wrapper.rs
            let mut source_path = input_path.clone();
            let source_file = source_path
                .file_name()
                .expect("not a file")
                .to_str()
                .unwrap()
                .to_owned();
            let _ = source_path.pop();
            source_path.push("wrapped_".to_owned() + &source_file);
            let mut wrapper_file = File::create(&source_path)?;
            wrapper_file.write_all(content.as_bytes())?;
            let dst = format!("include!({:?});", callbacks.source.input_path.clone());
            wrapper_file.write_all(dst.as_bytes())?;

            // construct new callback
            Ok(CompilerCallbacks {
                source: Source::new(Source::File { path: source_path }),
                is_parsing: false,
                functions: wrapped_functions,
                ..callbacks
            })
        }
        Input::Str { name, input } => match name {
            Custom(name) => Ok(CompilerCallbacks {
                source: Source::new(Source::Memory {
                    name,
                    code: content + &input,
                }),
                is_parsing: false,
                functions: wrapped_functions,
                ..callbacks
            }),
            _ => {
                unimplemented!()
            }
        },
    }
}
