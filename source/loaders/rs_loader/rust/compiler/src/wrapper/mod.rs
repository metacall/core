pub mod class;
use super::{config::Input, source_map::FileName::Custom, CompilerCallbacks, Function, Source};
use crate::Class;
use std::fs::File;
use std::io::Write;
use std::path::PathBuf;
fn generate_function_wrapper(functions: &Vec<Function>) -> String {
    let mut ret = String::new();
    for func in functions {
        ret.push_str(&format!(
            "#[no_mangle]\nunsafe fn metacall_register_fn_{}() -> *mut NormalFunction {{\n",
            func.name
        ));
        ret.push_str(&format!("\tlet f = NormalFunction::new({});\n", func.name));
        ret.push_str("\tBox::into_raw(Box::new(f))\n}\n");
    }
    ret
}

fn generate_class_wrapper(classes: &Vec<&crate::Class>) -> String {
    let mut ret = String::new();
    for class in classes {
        ret.push_str(&format!(
            "#[no_mangle]\nunsafe fn metacall_register_class_{}() -> *mut Class {{\n",
            class.name
        ));
        ret.push_str(&format!(
            "\tlet class = Class::builder::<{}>()\n",
            class.name
        ));
        // set constructor
        if let Some(_ctor) = &class.constructor {
            ret.push_str(&format!("\t\t.set_constructor({}::new)\n", class.name));
        } else {
            println!("there's no constructor in class: {}", class.name);
        }
        // set attributes
        for attr in &class.attributes {
            ret.push_str(&format!(
                "\t\t.add_attribute_getter(\"{}\", |f| f.{})\n",
                attr.name, attr.name
            ));
            ret.push_str(&format!(
                "\t\t.add_attribute_setter(\"{}\", |val, f| f.{} = val)\n",
                attr.name, attr.name
            ));
        }
        // set methods
        for method in &class.methods {
            ret.push_str(&format!(
                "\t\t.add_method(\"{}\", {}::{})\n",
                method.name, class.name, method.name
            ));
        }
        // set static methods
        for method in &class.static_methods {
            ret.push_str(&format!(
                "\t\t.add_class_method(\"{}\", {}::{})\n",
                method.name, class.name, method.name
            ));
        }
        // no need to set destructor
        ret.push_str("\t\t.build();\n");
        ret.push_str("\tBox::into_raw(Box::new(class))\n}\n");
    }
    ret
}

fn generate_function_wrapper_for_package(functions: &Vec<Function>) -> String {
    let mut ret = String::new();
    for func in functions {
        ret.push_str(&format!(
            "#[no_mangle]\nunsafe fn metacall_register_fn_{}() -> *mut NormalFunction {{\n",
            func.name
        ));
        ret.push_str(&format!(
            "\tlet f = NormalFunction::new(metacall_package::{});\n",
            func.name
        ));
        ret.push_str("\tBox::into_raw(Box::new(f))\n}\n");
    }
    ret
}
fn generate_class_wrapper_for_package(classes: &Vec<&crate::Class>) -> String {
    let mut ret = String::new();
    for class in classes {
        ret.push_str(&format!(
            "#[no_mangle]\nunsafe fn metacall_register_class_{}() -> *mut Class {{\n",
            class.name
        ));
        ret.push_str(&format!(
            "\tuse metacall_package::*;\nlet class = Class::builder::<{}>()\n",
            class.name
        ));
        // set constructor
        if let Some(_ctor) = &class.constructor {
            ret.push_str(&format!("\t\t.set_constructor({}::new)\n", class.name));
        } else {
            println!("there's no constructor in class: {}", class.name);
        }
        // set attributes
        for attr in &class.attributes {
            ret.push_str(&format!(
                "\t\t.add_attribute_getter(\"{}\", |f| f.{})\n",
                attr.name, attr.name
            ));
            ret.push_str(&format!(
                "\t\t.add_attribute_setter(\"{}\", |val, f| f.{} = val)\n",
                attr.name, attr.name
            ));
        }
        // set methods
        for method in &class.methods {
            ret.push_str(&format!(
                "\t\t.add_method(\"{}\", {}::{})\n",
                method.name, class.name, method.name
            ));
        }
        // set static methods
        for method in &class.static_methods {
            ret.push_str(&format!(
                "\t\t.add_class_method(\"{}\", {}::{})\n",
                method.name, class.name, method.name
            ));
        }
        // no need to set destructor
        ret.push_str("\t\t.build();\n");
        ret.push_str("\tBox::into_raw(Box::new(class))\n}\n");
    }
    ret
}
pub fn generate_wrapper(callbacks: CompilerCallbacks) -> std::io::Result<CompilerCallbacks> {
    match callbacks.source.source {
        Source::Package { path } => {
            let mut content = String::new();
            let function_wrapper = generate_function_wrapper_for_package(&callbacks.functions);
            content.push_str(&function_wrapper);
            let class_wrapper =
                generate_class_wrapper_for_package(&callbacks.classes.iter().collect());
            content.push_str(&class_wrapper);

            let source_dir = path.parent().expect("input path has no parent");

            // create metacall_class file
            println!("create: {:?}", source_dir.join("metacall_class.rs"));
            let mut class_file = File::create(source_dir.join("metacall_class.rs"))?;
            let bytes = include_bytes!("class.rs");
            class_file.write_all(bytes)?;
            println!("open: {:?}", source_dir.join("metacall_wrapped_package.rs"));
            let mut wrapper_file = std::fs::OpenOptions::new()
                .append(true)
                .open(source_dir.join("metacall_wrapped_package.rs"))?;
            // include class module
            wrapper_file.write_all(b"mod metacall_class;\nuse metacall_class::*;\n")?;
            wrapper_file.write_all(content.as_bytes())?;

            // construct new callback
            Ok(CompilerCallbacks {
                source: Source::new(Source::Package { path: path }),
                is_parsing: false,
                ..callbacks
            })
        }
        _ => {
            let mut content = String::new();
            let function_wrapper = generate_function_wrapper(&callbacks.functions);
            content.push_str(&function_wrapper);
            let class_wrapper = generate_class_wrapper(&callbacks.classes.iter().collect());
            content.push_str(&class_wrapper);

            match callbacks.source.input.0 {
                Input::File(input_path) => {
                    // generate wrappers to a file source_wrapper.rs
                    let mut source_path = input_path.clone();
                    let source_file = source_path
                        .file_name()
                        .expect("not a file")
                        .to_str()
                        .unwrap()
                        .to_owned();
                    let _ = source_path.pop();

                    // create metacall_class file
                    let mut class_file = File::create(source_path.join("metacall_class.rs"))?;
                    let bytes = include_bytes!("class.rs");
                    class_file.write_all(bytes)?;

                    source_path.push("wrapped_".to_owned() + &source_file);
                    let mut wrapper_file = File::create(&source_path)?;
                    // include class module
                    wrapper_file.write_all(b"mod metacall_class;\nuse metacall_class::*;\n")?;
                    wrapper_file.write_all(content.as_bytes())?;
                    let dst = format!("include!({:?});", callbacks.source.input_path.clone());
                    wrapper_file.write_all(dst.as_bytes())?;

                    // construct new callback
                    Ok(CompilerCallbacks {
                        source: Source::new(Source::File { path: source_path }),
                        is_parsing: false,
                        ..callbacks
                    })
                }
                Input::Str { name, input } => match name {
                    Custom(_name) => {
                        let source_path = std::env::temp_dir();
                        // write code to script
                        let mut source_file = File::create(source_path.join("script.rs"))?;
                        source_file.write_all(input.as_bytes())?;
                        // create metacall_class file
                        let mut class_file = File::create(source_path.join("metacall_class.rs"))?;
                        let bytes = include_bytes!("class.rs");
                        class_file.write_all(bytes)?;

                        // in order to solve the dependencies conflict,
                        // we use modules instead of putting them into a single file.
                        let mut wrapper_file = File::create(source_path.join("wrapped_script.rs"))?;
                        // include class module
                        wrapper_file.write_all(b"mod metacall_class;\nuse metacall_class::*;\n")?;
                        wrapper_file.write_all(content.as_bytes())?;
                        let dst = format!("include!({:?});", source_path.join("script.rs"));
                        wrapper_file.write_all(dst.as_bytes())?;

                        // construct new callback
                        Ok(CompilerCallbacks {
                            source: Source::new(Source::File {
                                path: source_path.join("wrapped_script.rs"),
                            }),
                            is_parsing: false,
                            ..callbacks
                        })
                    }
                    _ => {
                        unimplemented!()
                    }
                },
            }
        }
    }
}
