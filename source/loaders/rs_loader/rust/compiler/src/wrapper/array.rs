use crate::{FunctionParameter, FunctionType, Mutability, Reference};

use super::{value_to_rust_type, value_to_type, Wrapper};

#[derive(Debug)]
pub struct Vec {
    idx: usize,
    ty: FunctionParameter,
}

impl Vec {
    pub fn new(idx: usize, ty: FunctionParameter) -> Self {
        Self { idx, ty }
    }
}

impl Wrapper for Vec {
    fn as_arg(&self) -> String {
        match self.ty.mutability {
            Mutability::Yes => format!("{}: *mut c_void", self.arg_name()),
            Mutability::No => format!("{}: *mut c_void", self.arg_name()),
        }
    }

    fn arg_name(&self) -> String {
        format!("vec{}", self.idx)
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

        format!("{}{}r_vec{}", ref_symbol, mut_symbol, self.idx)
    }
    fn transform(&self, args_ptr: &str) -> String {
        let arr_ptr = format!("{}[{}]", args_ptr, self.idx);
        let idx = self.idx;
        let mut_symbol = {
            match self.ty.mutability {
                Mutability::Yes => "mut ",
                Mutability::No => "",
            }
        };
        match self.ty.reference {
            Reference::Yes => {
                format!(
                    "let arr{idx} = metacall_value_to_array({arr_ptr});
    let count{idx} = value_type_count({arr_ptr});
    let {mut_symbol}r_vec{idx} = 
        std::slice::from_raw_parts(arr{idx}, count{idx} as usize)
        .iter()
        .map(|p| {}(*p))
        .collect::<Vec<{}>>();\n",
                    value_to_type(&self.ty.generic[0].ty),
                    value_to_rust_type(&self.ty.generic[0].ty)
                )
            }
            Reference::No => {
                format!(
                    "let arr{idx} = metacall_value_to_array({arr_ptr});
    let count{idx} = value_type_count({arr_ptr});
    let {mut_symbol}r_vec{idx}= 
        std::slice::from_raw_parts(arr{idx}, count{idx} as usize)
        .iter()
        .map(|p| {}(*p))
        .collect::<Vec<{}>>()
        .clone();\n",
                    value_to_type(&self.ty.generic[0].ty),
                    value_to_rust_type(&self.ty.generic[0].ty)
                )
            }
        }
    }
    fn cleanup(&self) -> String {
        match self.ty.reference {
            Reference::Yes => {
                format!("std::mem::forget(r_vec{});\n", self.idx)
            }
            Reference::No => {
                format!("")
            }
        }
    }

    fn handle_ret(&self, ret_name: &str) -> String {
        format!("metacall_value_create_int({})", ret_name)
    }
    fn get_args_type(&self) -> FunctionParameter {
        FunctionParameter {
            name: self.arg_name(),
            mutability: self.ty.mutability.clone(),
            reference: Reference::No,
            ty: FunctionType::Array,
            generic: self.ty.generic.clone(),
        }
    }

    fn get_ret_type(&self) -> FunctionParameter {
        FunctionParameter { ..self.ty.clone() }
    }
}
