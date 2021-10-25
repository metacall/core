use std::{fs, path::PathBuf};

use parser::{self, ParsedSource};

pub struct FileRegistration {
    path_buf_to_file: PathBuf,
    parsed_source: ParsedSource,
}
impl FileRegistration {
    pub fn new(path_buf_to_file: PathBuf) -> FileRegistration {
        FileRegistration {
            path_buf_to_file: path_buf_to_file.clone(),
            parsed_source: parser::parse_functions(parser::Source::new(
                fs::read_to_string(path_buf_to_file.clone()).unwrap(),
                String::from(path_buf_to_file.file_name().unwrap().to_str().unwrap()),
            )),
        }
    }
}
