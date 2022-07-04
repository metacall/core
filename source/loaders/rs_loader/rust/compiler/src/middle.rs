use super::rustc_middle::ty::{
    subst::GenericArgKind, Binder, FloatTy, FnSig, IntTy, TyKind, TyS, UintTy,
};
use super::rustc_span::symbol::Ident;
use super::{Function, FunctionParameter, FunctionType, Mutability, Reference};
use std::iter::zip;

pub fn handle_ty(ty: &TyS) -> FunctionParameter {
    let mut result = FunctionParameter {
        name: String::new(),
        mutability: Mutability::No,
        reference: Reference::No,
        ty: FunctionType::Null,
        generic: vec![],
    };
    match &ty.kind() {
        TyKind::Int(i) => match i {
            IntTy::I16 => result.ty = FunctionType::I16,
            IntTy::I32 => result.ty = FunctionType::I32,
            IntTy::I64 => result.ty = FunctionType::I64,
            _ => result.ty = FunctionType::Null,
        },
        TyKind::Uint(u) => match u {
            UintTy::U16 => result.ty = FunctionType::U16,
            UintTy::U32 => result.ty = FunctionType::U32,
            UintTy::U64 => result.ty = FunctionType::U64,
            UintTy::Usize => result.ty = FunctionType::Usize,
            _ => result.ty = FunctionType::Null,
        },
        TyKind::Float(f) => match f {
            FloatTy::F32 => result.ty = FunctionType::F32,
            FloatTy::F64 => result.ty = FunctionType::F64,
        },
        TyKind::Bool => result.ty = FunctionType::Bool,
        TyKind::Char => result.ty = FunctionType::Char,
        TyKind::Str => result.ty = FunctionType::Str,
        TyKind::Adt(def, gen) => {
            let def_ident = format!("{:?}", def);
            match def_ident.as_str() {
                "std::vec::Vec" => {
                    result.ty = FunctionType::Array;
                    dbg!(gen);
                    let gen_arg = gen[0];
                    if let GenericArgKind::Type(ty) = gen_arg.unpack() {
                        result.generic.push(handle_ty(ty));
                    } else {
                        println!("expect generic arg, get nothing");
                    }
                }
                "std::collections::HashMap" => {
                    result.ty = FunctionType::Map;
                    let key = gen[0];
                    if let GenericArgKind::Type(ty) = key.unpack() {
                        result.generic.push(handle_ty(ty));
                    } else {
                        println!("expect key, get nothing");
                    }
                    let value = gen[1];
                    if let GenericArgKind::Type(ty) = value.unpack() {
                        result.generic.push(handle_ty(ty));
                    } else {
                        println!("expect value, get nothing");
                    }
                }
                "std::string::String" => result.ty = FunctionType::String,
                _ => result.ty = FunctionType::Null,
            }
        }
        _ => {}
    }
    result
}

pub fn handle_fn(name: String, sig: &Binder<FnSig>, names: &[Ident]) -> Function {
    let mut function = Function {
        name,
        ret: None,
        args: vec![],
    };
    // parse input and output
    let inputs = sig.inputs().skip_binder();
    for (name, ty) in zip(names, inputs) {
        let mut func_parameter = handle_ty(ty);
        func_parameter.name = name.to_string();
        function.args.push(func_parameter);
    }
    let output = sig.output().skip_binder();
    match output.kind() {
        TyKind::Tuple(arg) => {
            // default return
            if arg.len() == 0 {
                function.ret = None;
            } else {
                function.ret = Some(handle_ty(output));
            }
        }
        _ => {
            function.ret = Some(handle_ty(output));
        }
    }
    function
}
