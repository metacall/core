use crate::{Parser, RegistrationError};

use std::{
    ffi::{c_void, OsStr},
    path::PathBuf,
};

use cargo_toml::Manifest as CargoTomlMainfest;

#[derive(Debug)]
pub struct PackageRegistration {
    parser: Parser,
    path_to_librs: PathBuf,
    path_to_cargotoml: PathBuf,
    path_to_project_root: PathBuf,
}
impl PackageRegistration {
    fn validate_cargo_project_is_cdylib(
        path_to_project_root: &PathBuf,
        path_to_cargotoml: &PathBuf,
    ) -> Result<PathBuf, String> {
        let invalid_cdylib_cargo_project_error_message = format!(
            "Not a cdylib cargo project: {}",
            path_to_cargotoml.to_str().unwrap()
        );

        let cargo_toml_mainfest_from_pathresult = CargoTomlMainfest::from_path(path_to_cargotoml);

        match cargo_toml_mainfest_from_pathresult {
            Ok(cargo_toml_mainfest) => match cargo_toml_mainfest.lib {
                Some(cargo_toml_mainfest_lib) => {
                    if cargo_toml_mainfest_lib
                        .crate_type
                        .contains(&String::from("cdylib"))
                    {
                        let mut path_to_librs = path_to_project_root.clone();

                        path_to_librs.push("src");
                        path_to_librs.push("lib");
                        path_to_librs.set_extension("rs");

                        if path_to_librs.exists() && path_to_librs.is_file() {
                            return Ok(path_to_librs);
                        }

                        return Err(format!("Couldn't find the path to src/lib.rs"));
                    }

                    return Err(invalid_cdylib_cargo_project_error_message);
                }
                None => return Err(invalid_cdylib_cargo_project_error_message),
            },
            Err(error) => return Err(error.to_string()),
        }
    }
    fn validate_path_to_cargotoml(path_to_cargotoml: &PathBuf) -> Result<PathBuf, String> {
        if path_to_cargotoml.exists()
            && path_to_cargotoml.is_dir()
            && path_to_cargotoml.file_name() == Some(OsStr::new("Cargo.toml"))
        {
            let mut path_to_project_root = path_to_cargotoml.clone();

            path_to_project_root.pop();

            if path_to_cargotoml.is_dir() {
                return Ok(path_to_project_root);
            }

            return Err(String::from("Couldn't locate project's root directory"));
        }

        return Err(String::from("Not the path to a Cargo.toml file"));
    }

    fn validation(path_to_cargotoml: PathBuf) -> Result<(PathBuf, PathBuf, PathBuf), String> {
        let path_to_project_root =
            PackageRegistration::validate_path_to_cargotoml(&path_to_cargotoml)?;

        let path_to_librs = PackageRegistration::validate_cargo_project_is_cdylib(
            &path_to_project_root,
            &path_to_cargotoml,
        )?;

        return Ok((path_to_project_root, path_to_cargotoml, path_to_librs));
    }

    pub fn new(path_to_cargotoml: PathBuf) -> Result<PackageRegistration, RegistrationError> {
        let (path_to_project_root, path_to_cargotoml, path_to_librs) =
            match PackageRegistration::validation(path_to_cargotoml) {
                Ok(instance) => instance,
                Err(error) => return Err(RegistrationError::ValidationError(error)),
            };

        let parser = match Parser::new(&path_to_librs) {
            Ok(instance) => instance,
            Err(error) => return Err(RegistrationError::SynError(error)),
        };

        Ok(PackageRegistration {
            parser,
            path_to_librs,
            path_to_cargotoml,
            path_to_project_root,
        })
    }

    pub fn register_in_metacall(&self, _ctx: *mut c_void) -> Result<(), String> {
        Ok(())
    }
}
