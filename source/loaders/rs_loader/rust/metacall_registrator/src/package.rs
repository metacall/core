use std::{ffi::OsStr, path::PathBuf};

use cargo_toml::Manifest as CargoTomlMainfest;

pub struct PackageRegistration {
    path_buf_to_project_root_directory: PathBuf,
    path_buf_to_librs: PathBuf,
}
impl PackageRegistration {
    fn get_path_buf_to_project_root_directory_from_librs_path_buf(
        path_buf_to_librs: PathBuf,
    ) -> Result<PathBuf, String> {
        let mut path_buf_to_project_root_directory = path_buf_to_librs;

        path_buf_to_project_root_directory =
            PathBuf::from(match path_buf_to_project_root_directory.parent() {
                Some(path_buf_to_project_root_directory_parent1) => {
                    match path_buf_to_project_root_directory_parent1.parent() {
                        Some(path_buf_to_project_root_directory_parent2) => {
                            path_buf_to_project_root_directory_parent2
                        }
                        None => return Err(String::from("Unable to find the root of the project")),
                    }
                }
                None => return Err(String::from("Unable to find the root of the project")),
            });

        Ok(path_buf_to_project_root_directory)
    }
    fn get_path_buf_to_project_cargotoml_from_project_root_directory_path_buf(
        path_buf_to_project_root_directory: PathBuf,
    ) -> Result<PathBuf, String> {
        let mut path_buf_to_project_cargotoml = path_buf_to_project_root_directory;

        path_buf_to_project_cargotoml.push("Cargo");
        path_buf_to_project_cargotoml.set_extension("toml");

        if !path_buf_to_project_cargotoml.exists() || !path_buf_to_project_cargotoml.is_file() {
            return Err(format!(
                "Cargo.toml not found in the cargo project's root. No such file found: {}",
                path_buf_to_project_cargotoml.to_str().unwrap()
            ));
        }

        Ok(path_buf_to_project_cargotoml)
    }
    fn validate_cargo_project_is_cdylib(
        path_buf_to_project_root_directory: PathBuf,
    ) -> Result<(), String> {
        let path_buf_to_project_cargotoml = PackageRegistration::get_path_buf_to_project_cargotoml_from_project_root_directory_path_buf(path_buf_to_project_root_directory)?;
        let invalid_cdylib_cargo_project_error_message = format!(
            "Not a cdylib cargo project: {}",
            path_buf_to_project_cargotoml.to_str().unwrap()
        );

        let cargo_toml_mainfest_from_pathresult =
            CargoTomlMainfest::from_path(path_buf_to_project_cargotoml);

        match cargo_toml_mainfest_from_pathresult {
            Ok(cargo_toml_mainfest) => match cargo_toml_mainfest.lib {
                Some(cargo_toml_mainfest_lib) => {
                    if cargo_toml_mainfest_lib
                        .crate_type
                        .contains(&String::from("cdylib"))
                    {
                        return Ok(());
                    }

                    return Err(invalid_cdylib_cargo_project_error_message);
                }
                None => return Err(invalid_cdylib_cargo_project_error_message),
            },
            Err(error) => return Err(error.to_string()),
        }
    }

    fn validation(path_buf_to_librs: PathBuf) -> Result<(PathBuf, PathBuf), String> {
        if path_buf_to_librs.file_name() == Some(OsStr::new("lib.rs")) {
            let mut path_buf_to_project_root_directory =
                PackageRegistration::get_path_buf_to_project_root_directory_from_librs_path_buf(
                    path_buf_to_librs.clone(),
                )?;

            let path_buf_to_project_cargotoml =
            PackageRegistration::get_path_buf_to_project_cargotoml_from_project_root_directory_path_buf(path_buf_to_project_root_directory.clone())?;

            PackageRegistration::validate_cargo_project_is_cdylib(path_buf_to_project_cargotoml)?;

            return Ok((path_buf_to_project_root_directory, path_buf_to_librs));
        }

        return Err(String::from("Not a path to a lib.rs file"));
    }

    pub fn new(path_buf_to_librs: PathBuf) -> Result<PackageRegistration, String> {
        let (path_buf_to_project_root_directory, path_buf_to_librs) =
            PackageRegistration::validation(path_buf_to_librs)?;

        Ok(PackageRegistration {
            path_buf_to_project_root_directory,
            path_buf_to_librs,
        })
    }

    pub fn register_the_project_in_metacall(&self) -> Result<PackageRegistration, String> {
        Err(String::from(
            "Basic implementation from 'register_the_project_in_metacall'",
        ))
    }
}
