use super::{value_create_type, Wrapper};
use crate::{FunctionParameter, Mutability, Reference};

#[derive(Debug, Clone)]
pub struct MString {
    idx: usize,
    ty: FunctionParameter,
}

impl MString {
    pub fn new(idx: usize, ty: FunctionParameter) -> Self {
        Self { idx, ty }
    }
}

impl Wrapper for MString {
    fn as_arg(&self) -> String {
        format!("{}: *mut c_void, ", self.arg_name())
    }
    fn arg_name(&self) -> String {
        format!("string{}", self.idx)
    }
    fn var_name(&self) -> String {
        let mut_symbol = {
            match self.ty.mutability {
                Mutability::Yes => "mut ",
                Mutability::No => "",
            }
        };
        let ref_symbol = {
            match self.ty.reference {
                Reference::Yes => "&",
                Reference::No => "",
            }
        };

        format!("{}{}var_string{}", ref_symbol, mut_symbol, self.idx)
    }

    fn transform(&self, args_ptr: &str) -> String {
        // let var_string0: *mut c_char = metacall_value_to_string(args_ptr[0]);
        // let {mut} var_string0: String = CStr::from_ptr(var_string0).to_str().unwrap().to_owned();
        let arr_ptr = format!("{}[{}]", args_ptr, self.idx);
        let idx = self.idx;
        let var_name = format!("var_string{idx}");
        let mut_symbol = {
            match self.ty.mutability {
                Mutability::Yes => "mut ",
                Mutability::No => "",
            }
        };
        format!(
            "let {var_name} = metacall_value_to_string({arr_ptr});
    let {mut_symbol}{var_name} = CStr::from_ptr({var_name}).to_str().unwrap().to_owned();"
        )
    }

    fn cleanup(&self) -> String {
        // we need to drop strings here.
        format!("\n")
    }

    fn handle_ret(&self, ret_name: &str) -> String {
        // the data will be copied, use as_ptr instead of into_raw
        format!("metacall_value_create_string({ret_name}.as_ptr() as *const i8, {ret_name}.len())",)
    }
    fn get_args_type(&self) -> FunctionParameter {
        FunctionParameter {
            name: self.arg_name(),
            ..self.ty.clone()
        }
    }

    fn get_ret_type(&self) -> FunctionParameter {
        FunctionParameter { ..self.ty.clone() }
    }
}
