use super::rustc_ast::{
    AngleBracketedArg, AngleBracketedArgs, FnRetTy, FnSig, GenericArg, GenericArgs, MutTy, Pat,
    PatKind, TyKind,
};
use super::{Function, FunctionParameter, FunctionType, Mutability, Reference};

pub fn handle_ty(ty: &rustc_ast::Ty) -> FunctionParameter {
    let mut result = FunctionParameter {
        name: String::new(),
        mutability: Mutability::No,
        reference: Reference::No,
        ty: FunctionType::Null,
        generic: vec![],
    };
    match &ty.kind {
        TyKind::Path(_, path) => {
            let segment = &path.segments[0];
            let symbol_string = segment.ident.name.to_string();
            match symbol_string.as_str() {
                "i16" => result.ty = FunctionType::I16,
                "i32" => result.ty = FunctionType::I32,
                "i64" => result.ty = FunctionType::I64,
                "u16" => result.ty = FunctionType::U16,
                "u32" => result.ty = FunctionType::U32,
                "u64" => result.ty = FunctionType::U64,
                "usize" => result.ty = FunctionType::Usize,
                "f32" => result.ty = FunctionType::F32,
                "f64" => result.ty = FunctionType::F64,
                "bool" => result.ty = FunctionType::Bool,
                "str" => result.ty = FunctionType::Str,
                "Vec" => {
                    result.ty = FunctionType::Array;
                    if let Some(args) = &segment.args {
                        match &**args {
                            GenericArgs::AngleBracketed(AngleBracketedArgs { args, .. }) => {
                                for arg in args {
                                    match arg {
                                        AngleBracketedArg::Arg(GenericArg::Type(ty)) => {
                                            result.generic.push(handle_ty(ty))
                                        }
                                        _ => {}
                                    }
                                }
                            }
                            GenericArgs::Parenthesized(_) => {}
                        }
                    }
                }
                "HashMap" => {
                    result.ty = FunctionType::Map;
                    if let Some(args) = &segment.args {
                        match &**args {
                            GenericArgs::AngleBracketed(AngleBracketedArgs { args, .. }) => {
                                for arg in args {
                                    match arg {
                                        AngleBracketedArg::Arg(GenericArg::Type(ty)) => {
                                            result.generic.push(handle_ty(ty))
                                        }
                                        _ => {}
                                    }
                                }
                            }
                            GenericArgs::Parenthesized(_) => {}
                        }
                    }
                }
                "String" => result.ty = FunctionType::String,
                _ => {}
            }
            result.name = symbol_string;
        }
        TyKind::Rptr(_, MutTy { ty, mutbl }) => {
            let mut inner_ty = handle_ty(ty);
            inner_ty.reference = Reference::Yes;
            match mutbl {
                rustc_hir::Mutability::Mut => inner_ty.mutability = Mutability::Yes,
                rustc_hir::Mutability::Not => inner_ty.mutability = Mutability::No,
            }
            return inner_ty;
        }
        TyKind::ImplicitSelf => {
            result.name = "self".to_string();
            result.ty = FunctionType::This
        }
        _ => {}
    }
    result
}

fn handle_pat(pat: &Pat) -> Option<String> {
    match pat.kind {
        PatKind::Ident(_, ident, _) => return Some(ident.name.to_string()),
        _ => {}
    }
    None
}

pub fn handle_fn(name: String, sig: &FnSig) -> Function {
    let mut function = Function {
        name,
        ret: None,
        args: vec![],
    };
    // parse input and output
    for arg in &sig.decl.inputs {
        let mut param = handle_ty(&arg.ty);
        // we need to extract the name from pat.
        if let Some(name) = handle_pat(&arg.pat) {
            param.name = name;
        }
        function.args.push(param);
    }

    match &sig.decl.output {
        FnRetTy::Default(_) => function.ret = None,
        FnRetTy::Ty(ty) => {
            function.ret = Some(handle_ty(&ty));
        }
    }
    function
}
