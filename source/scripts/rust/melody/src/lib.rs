use serde_json::Value;

pub fn compile(s: String) -> String {
    let v: Value = serde_json::from_str(&s).unwrap();
    v["name"].to_string()
}
