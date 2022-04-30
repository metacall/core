use crate::FunctionParameter;

use super::{value_create_type, value_to_type, Wrapper};

#[derive(Debug, Clone)]
pub struct Number {
    idx: usize,
    ty: FunctionParameter,
}

impl Number {
    pub fn new(idx: usize, ty: FunctionParameter) -> Self {
        Self { idx, ty }
    }
}

impl Wrapper for Number {
    fn as_arg(&self) -> String {
        format!("{}: *mut c_void, ", self.arg_name())
    }
    fn arg_name(&self) -> String {
        format!("num{}", self.idx)
    }
    fn var_name(&self) -> String {
        format!("var_num{}", self.idx)
    }
    fn transform(&self, args_ptr: &str) -> String {
        format!(
            "let {} = {}({}[{}]);\n",
            self.var_name(),
            value_to_type(&self.ty.ty),
            args_ptr,
            self.idx
        )
    }
    fn cleanup(&self) -> String {
        format!("\n")
    }

    fn handle_ret(&self, ret_name: &str) -> String {
        format!("{}({})", value_create_type(&self.ty.ty), ret_name)
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
