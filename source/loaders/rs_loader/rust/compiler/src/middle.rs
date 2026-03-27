use crate::Attribute;

use super::rustc_middle::ty::{
    self, FloatTy, IntTy, Ty, TyCtxt, TyKind, UintTy, Visibility,
};
use super::rustc_span::symbol::Ident;
use super::{Function, FunctionParameter, FunctionType, Mutability, Reference};
use rustc_hir::def::{DefKind, Res};
use rustc_middle::metadata::ModChild;
use std::iter::zip;

// Updated: TyS removed in modern rustc, use Ty<'tcx> instead
pub fn handle_ty<'tcx>(ty: Ty<'tcx>) -> FunctionParameter {
    let mut result = FunctionParameter {
        name: String::new(),
        mutability: Mutability::No,
        reference: Reference::No,
        ty: FunctionType::Null,
        generic: vec![],
    };
    match ty.kind() {
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
            _ => result.ty = FunctionType::Null,
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
                    if let ty::GenericArgKind::Type(ty) = gen_arg.unpack() {
                        result.generic.push(handle_ty(ty));
                    } else {
                        eprintln!("Rust Loader: Expect generic arg, get nothing");
                    }
                }
                "std::collections::HashMap" => {
                    result.ty = FunctionType::Map;
                    let key = gen[0];
                    if let ty::GenericArgKind::Type(ty) = key.unpack() {
                        result.generic.push(handle_ty(ty));
                    } else {
                        eprintln!("Rust Loader: Expect key, get nothing");
                    }
                    let value = gen[1];
                    if let ty::GenericArgKind::Type(ty) = value.unpack() {
                        result.generic.push(handle_ty(ty));
                    } else {
                        eprintln!("Rust Loader: Expect value, get nothing");
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

// Updated: sig is now PolyFnSig (Binder<FnSig>) after skip_binder on EarlyBinder
pub fn handle_fn<'tcx>(name: String, sig: &ty::Binder<'tcx, ty::FnSig<'tcx>>, names: &[Ident]) -> Function {
    let mut function = Function {
        name,
        ret: None,
        args: vec![],
    };
    // parse input and output
    let inputs = sig.skip_binder().inputs();
    for (name, ty) in zip(names, inputs) {
        let mut func_parameter = handle_ty(*ty);
        func_parameter.name = name.to_string();
        function.args.push(func_parameter);
    }
    let output = sig.skip_binder().output();
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

// Updated: Export replaced with ModChild
pub fn extract_attribute_from_mod_child(ctxt: &TyCtxt, child: &ModChild) -> Option<Attribute> {
    let ident = child.ident;
    let res = child.res;
    let vis = child.vis;
    // skip non-public items
    if !matches!(vis, Visibility::Public) {
        return None;
    }
    match res {
        Res::Def(DefKind::Field, def_id) => Some(Attribute {
            name: ident.to_string(),
            // Updated: type_of returns EarlyBinder, need skip_binder/instantiate_identity
            ty: handle_ty(ctxt.type_of(def_id).instantiate_identity()),
        }),
        _ => None,
    }
}

// Updated: Export replaced with ModChild
pub fn extract_fn_from_mod_child(ctxt: &TyCtxt, child: &ModChild) -> Option<Function> {
    let ident = child.ident;
    let res = child.res;
    let vis = child.vis;
    // skip non-public items
    if !matches!(vis, Visibility::Public) {
        return None;
    }
    match res {
        Res::Def(DefKind::AssocFn, def_id) => {
            // Updated: fn_sig returns EarlyBinder, skip_binder to get PolyFnSig
            let fn_sig = ctxt.fn_sig(def_id).skip_binder();
            let names = ctxt.fn_arg_names(def_id);
            Some(handle_fn(ident.to_string(), &fn_sig, names))
        }
        _ => None,
    }
}
