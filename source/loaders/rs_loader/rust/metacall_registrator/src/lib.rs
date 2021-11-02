use std::{error::Error, fs::File, io::Read, path::PathBuf};

pub use syn::{self, File as SynFileAst};

pub mod file;
pub mod package;
pub(crate) mod registrator;

pub enum RegistrationError {
    DlopenError(String),
    ValidationError(String),
    SynError(Box<dyn Error>),
}

#[derive(Debug)]
pub struct Parser {
    pub code: String,
    pub ast: SynFileAst,
}
impl Parser {
    pub fn new(path_to_code: &PathBuf) -> Result<Parser, Box<dyn Error>> {
        let mut file = File::open(path_to_code)?;

        let mut content = String::new();
        file.read_to_string(&mut content)?;

        let code = content.clone();
        let ast: SynFileAst = syn::parse_file(&content)?;

        Ok(Parser { ast, code })
    }
}
