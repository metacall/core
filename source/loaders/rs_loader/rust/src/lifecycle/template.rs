use compiler::TemplateInstantiation;
use metacall::MetaCallValue;
use std::os::raw::c_void;

/// Templates to instantiate at load time, the load data has the format:
/// { "templates": [ { "identity": [ "i64" ] }, { "pair": [ "i32", "i64" ] } ] }
pub fn instantiations_from_data(data: *mut c_void) -> Result<Vec<TemplateInstantiation>, String> {
    if data.is_null() {
        return Ok(Vec::new());
    }

    // The data is owned by the caller, so it is converted without destroying it
    let data = <Box<dyn MetaCallValue>>::from_metacall_raw_leak(data)
        .map_err(|_| "Rust loader received invalid load data.".to_owned())?;
    let data = data
        .as_map()
        .ok_or("Rust loader load data must be a map.")?;

    let Some(templates) = data.get("templates") else {
        return Ok(Vec::new());
    };

    templates
        .as_array()
        .ok_or("Rust loader load data 'templates' must be an array.")?
        .iter()
        .enumerate()
        .map(|(index, template)| instantiation_from_value(index, &**template))
        .collect()
}

fn instantiation_from_value(
    index: usize,
    value: &(dyn MetaCallValue + 'static),
) -> Result<TemplateInstantiation, String> {
    let map = value
        .as_map()
        .ok_or_else(|| format!("Template at index {} must be a map.", index))?;
    let mut entries = map.iter();

    let (Some((template, types)), None) = (entries.next(), entries.next()) else {
        return Err(format!(
            "Template at index {} must have exactly one name.",
            index
        ));
    };

    let types = types
        .as_array()
        .ok_or_else(|| format!("Template '{}' types must be an array.", template))?
        .iter()
        .map(|ty| type_from_value(template, &**ty))
        .collect::<Result<Vec<String>, String>>()?;

    Ok(TemplateInstantiation {
        template: template.clone(),
        types,
    })
}

fn type_from_value(
    template: &str,
    value: &(dyn MetaCallValue + 'static),
) -> Result<String, String> {
    if let Some(ty) = value.as_str() {
        return Ok(ty.to_owned());
    }

    // TODO: Nested template arguments need class templates (see TemplateInstantiation)
    if value.as_map().is_some() {
        return Err(format!(
            "Template '{}' has a nested template argument, which is not supported yet.",
            template
        ));
    }

    Err(format!(
        "Template '{}' has an argument that is not a string.",
        template
    ))
}
