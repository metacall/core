#[allow(unused_imports)]
use crate::Function;

#[derive(Debug, Clone)]
pub enum TemplateKind {
    Function,
    Class,
}

#[derive(Debug, Clone)]
pub enum Type {
    I32,
    String,
    Bool,
}

#[derive(Debug, Clone)]
pub struct Template {
    pub name: String,
    pub kind: TemplateKind,
    pub parameters: Vec<String>,
}

pub fn template_instantiate_function(template: &Template, type_list: Vec<Type>) -> String {
    format!("{}::<{:?}>", template.name, type_list)
}

pub fn template_instantiate_class(template: &Template, type_list: Vec<Type>) -> String {
    format!("{}::<{:?}>", template.name, type_list)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_function_template_instantiation() {
        let template = Template {
            name: "identity".to_string(),
            kind: TemplateKind::Function,
            parameters: vec!["T".to_string()],
        };

        let result = template_instantiate_function(&template, vec![Type::I32]);

        assert!(result.contains("identity"));
    }

    #[test]
    fn test_class_template_instantiation() {
        let template = Template {
            name: "Wrapper".to_string(),
            kind: TemplateKind::Class,
            parameters: vec!["T".to_string()],
        };

        let result = template_instantiate_class(&template, vec![Type::I32]);

        assert!(result.contains("Wrapper"));
    }

    #[test]
    fn test_function_struct_template_instantiation() {
        let function = Function {
            name: "identity".to_string(),
            ret: None,
            args: vec![],
            generics: vec!["T".to_string()],
        };

        let instantiated = function.instantiate(vec!["i32".to_string()]);

        assert_eq!(instantiated.name, "identity::<i32>");

        assert!(instantiated.generics.is_empty());
    }
}
