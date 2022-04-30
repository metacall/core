use super::{value_to_rust_type, value_to_type, Wrapper};
use crate::{FunctionParameter, FunctionType, Mutability, Reference};

#[derive(Debug, Clone)]
pub struct Map {
    idx: usize,
    ty: FunctionParameter,
}

impl Map {
    pub fn new(idx: usize, ty: FunctionParameter) -> Self {
        Self { idx, ty }
    }
}

impl Wrapper for Map {
    fn as_arg(&self) -> String {
        format!("{}: *mut c_void, ", self.arg_name())
    }
    fn arg_name(&self) -> String {
        format!("map{}", self.idx)
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
        format!("{}{}r_map{}", ref_symbol, mut_symbol, self.idx)
    }
    fn transform(&self, args_ptr: &str) -> String {
        let map_ptr = format!("{}[{}]", args_ptr, self.idx);
        let idx = self.idx;
        let m_map = format!("m_map{idx}");
        format!(
            "let {m_map} = metacall_value_to_map({map_ptr});
        let count_{idx} = value_type_count({map_ptr});
        let {m_map} = std::slice::from_raw_parts({m_map}, count_{idx} as usize);
        let mut r_map{idx}: HashMap<{}, {}> = HashMap::new();
        for map_value in {m_map} {{
            let m_pair = metacall_value_to_array(*map_value);
            let m_pair = std::slice::from_raw_parts(m_pair, 2);
            let key = {}(m_pair[0]);
            let val = {}(m_pair[1]);
            r_map{idx}.insert(key, val);
        }}\n",
            value_to_rust_type(&self.ty.generic[0].ty),
            value_to_rust_type(&self.ty.generic[1].ty),
            value_to_type(&self.ty.generic[0].ty),
            value_to_type(&self.ty.generic[1].ty)
        )
    }
    fn cleanup(&self) -> String {
        match self.ty.reference {
            Reference::Yes => {
                format!("std::mem::forget(r_map{});\n", self.idx)
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
            ty: FunctionType::Map,
            generic: self.ty.generic.clone(),
        }
    }

    fn get_ret_type(&self) -> FunctionParameter {
        FunctionParameter { ..self.ty.clone() }
    }
}
