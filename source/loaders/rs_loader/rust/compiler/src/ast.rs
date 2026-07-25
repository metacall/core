use super::rustc_ast::{
    AngleBracketedArg, AngleBracketedArgs, FnRetTy, FnSig, GenericArg, GenericArgs, MutTy, Pat,
    PatKind, TyKind,
};
use super::{Function, FunctionParameter, FunctionType, Mutability, Reference};

pub fn handle_ty(ty: &rustc_ast::Ty, generics: &Vec<String>) -> FunctionParameter {
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

            if generics.contains(&symbol_string) {
                result.ty = FunctionType::Complex;
                result.name = symbol_string;
                return result;
            }
            match symbol_string.as_str() {
                "i16" => result.ty = FunctionType::i16,
                "i32" => result.ty = FunctionType::i32,
                "i64" => result.ty = FunctionType::i64,
                "u16" => result.ty = FunctionType::u16,
                "u32" => result.ty = FunctionType::u32,
                "u64" => result.ty = FunctionType::u64,
                "usize" => result.ty = FunctionType::usize,
                "f32" => result.ty = FunctionType::f32,
                "f64" => result.ty = FunctionType::f64,
                "bool" => result.ty = FunctionType::bool,
                "i8" => result.ty = FunctionType::char,
                "str" => result.ty = FunctionType::String,
                "Vec" => {
                    result.ty = FunctionType::Array;
                    if let Some(args) = &segment.args {
                        match &**args {
                            GenericArgs::AngleBracketed(AngleBracketedArgs { args, .. }) => {
                                for arg in args {
                                    if let AngleBracketedArg::Arg(GenericArg::Type(ty)) = arg {
                                        result.generic.push(handle_ty(ty, generics))
                                    }
                                }
                            }
                            GenericArgs::Parenthesized(_) => {}
                            GenericArgs::ParenthesizedElided(_) => {}
                        }
                    }
                }
                "HashMap" => {
                    result.ty = FunctionType::Map;
                    if let Some(args) = &segment.args {
                        match &**args {
                            GenericArgs::AngleBracketed(AngleBracketedArgs { args, .. }) => {
                                for arg in args {
                                    if let AngleBracketedArg::Arg(GenericArg::Type(ty)) = arg {
                                        result.generic.push(handle_ty(ty, generics))
                                    }
                                }
                            }
                            GenericArgs::Parenthesized(_) => {}
                            GenericArgs::ParenthesizedElided(_) => {}
                        }
                    }
                }
                "String" => result.ty = FunctionType::String,
                _ => {}
            }
            result.name = symbol_string;
        }
        TyKind::Ref(_, MutTy { ty, mutbl }) => {
            let mut inner_ty = handle_ty(ty, generics);
            inner_ty.reference = Reference::Yes;
            match mutbl {
                rustc_ast::Mutability::Mut => inner_ty.mutability = Mutability::Yes,
                rustc_ast::Mutability::Not => inner_ty.mutability = Mutability::No,
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
    if let PatKind::Ident(_, ident, _) = pat.kind {
        return Some(ident.name.to_string());
    }
    None
}

pub fn handle_fn(name: String, sig: &FnSig, generics: &rustc_ast::Generics) -> Function {
    let generics_params = generics
        .params
        .iter()
        .map(|param| param.ident.name.to_string())
        .collect::<Vec<String>>();

    let mut function = Function {
        name,
        ret: None,
        args: vec![],
        generics: generics_params.clone(),
    };
    // parse input and output
    for arg in &sig.decl.inputs {
        let mut param = handle_ty(&arg.ty, &generics_params);
        // we need to extract the name from pat.
        if let Some(name) = handle_pat(&arg.pat) {
            param.name = name;
        }
        function.args.push(param);
    }

    match &sig.decl.output {
        FnRetTy::Default(_) => function.ret = None,
        FnRetTy::Ty(ty) => {
            function.ret = Some(handle_ty(ty, &generics_params));
        }
    }
    function
}
