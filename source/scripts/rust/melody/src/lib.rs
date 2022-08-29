use melody_compiler::compiler;

pub fn compile(s: String) -> String {
    let ret = compiler(&s).unwrap_or_else(|x| format!("Compiler error {:?}", x));
    dbg!(&ret);
    ret
}
