use crate::Attribute;

use super::rustc_middle::ty::{
    subst::GenericArgKind, Binder, FloatTy, FnSig, IntTy, TyCtxt, TyKind, TyS, UintTy, Visibility,
};
use super::rustc_span::symbol::Ident;
use super::{Function, FunctionParameter, FunctionType, Mutability, Reference};
use rustc_hir::def::{DefKind, Res};
use rustc_middle::hir::exports::Export;
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
            IntTy::I16 => result.ty = FunctionType::i16,
            IntTy::I32 => result.ty = FunctionType::i32,
            IntTy::I64 => result.ty = FunctionType::i64,
            _ => result.ty = FunctionType::Null,
        },
        TyKind::Uint(u) => match u {
            UintTy::U16 => result.ty = FunctionType::u16,
            UintTy::U32 => result.ty = FunctionType::u32,
            UintTy::U64 => result.ty = FunctionType::u64,
            UintTy::Usize => result.ty = FunctionType::usize,
            _ => result.ty = FunctionType::Null,
        },
        TyKind::Float(f) => match f {
            FloatTy::F32 => result.ty = FunctionType::f32,
            FloatTy::F64 => result.ty = FunctionType::f64,
        },
        TyKind::Bool => result.ty = FunctionType::bool,
        TyKind::Char => result.ty = FunctionType::char,
        TyKind::Str => result.ty = FunctionType::str,
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

pub fn extract_attribute_from_export(ctxt: &TyCtxt, export: &Export) -> Option<Attribute> {
    let Export {
        ident, res, vis, ..
    } = export;
    // skip non-public items
    if !matches!(vis, Visibility::Public) {
        return None;
    }
    match res {
        Res::Def(DefKind::Field, def_id) => Some(Attribute {
            name: ident.to_string(),
            ty: handle_ty(ctxt.type_of(*def_id)),
        }),
        _ => None,
    }
}

pub fn extract_fn_from_export(ctxt: &TyCtxt, export: &Export) -> Option<Function> {
    let Export {
        ident, res, vis, ..
    } = export;
    // skip non-public items
    if !matches!(vis, Visibility::Public) {
        return None;
    }
    match res {
        Res::Def(DefKind::AssocFn, def_id) => {
            let fn_sig = ctxt.fn_sig(*def_id);
            let names = ctxt.fn_arg_names(*def_id);
            Some(handle_fn(ident.to_string(), &fn_sig, names))
        }
        _ => None,
    }
}
