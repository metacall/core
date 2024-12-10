#[cfg(test)]
mod tests {
    use metacall::{load, LoaderError};
    use std::env;

    #[test]
    fn invalid_loaders() {
        let scripts_dir = env::current_dir().unwrap().join("tests/scripts");
        let inavlid_file = scripts_dir.join("whatever.yeet");
        let valid_file = scripts_dir.join("script.js");

        if let Err(LoaderError::FileNotFound(_)) =
            load::from_single_file("random", inavlid_file) {
            // Everything Ok
        } else {
            panic!("Expected the loader fail with `FileNotFound` error variant!");
        }

        if let Err(LoaderError::FromFileFailure) =
            load::from_single_file("random", valid_file) {
            // Everything Ok
        } else {
            panic!("Expected the loader fail with `FromFileFailure` error variant!");
        }

        if let Err(LoaderError::FromMemoryFailure) =
            load::from_memory("random", "Invalid code!") {
            // Everything Ok
        } else {
            panic!("Expected the loader fail with `FromMemoryFailure` error variant!");
        }
    }
}
