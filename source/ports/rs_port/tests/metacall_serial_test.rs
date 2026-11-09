use metacall::{deserialize, initialize, is_initialized, serialize, MetaCallNull, MetaCallValue};

const TEMPLATES: &str = r#"{ "templates": [ { "identity": [ "i64" ] }, { "identity": [ "i32" ] }, { "pair": [ "i32", "i64" ] }, { "Recursive": [ "i32", { "Inner": [ "i64", "i64" ] } ] } ] }"#;

#[test]
fn metacall_serial() {
    let _d = initialize().unwrap();

    assert!(is_initialized());

    let value = deserialize::<Box<dyn MetaCallValue>>(TEMPLATES).unwrap();
    let json = serialize(value.clone()).unwrap();
    let round_trip = deserialize::<Box<dyn MetaCallValue>>(&json).unwrap();

    for value in [&value, &round_trip] {
        let templates = &value["templates"];

        assert_eq!(templates.as_array().unwrap().len(), 4);

        assert_eq!(templates[0]["identity"][0], "i64");
        assert_eq!(templates[1]["identity"][0], "i32");
        assert_eq!(templates[2]["pair"][0], "i32");
        assert_eq!(templates[2]["pair"][1], "i64");
        assert_eq!(templates[3]["Recursive"][0], "i32");
        assert_eq!(templates[3]["Recursive"][1]["Inner"][0], "i64");
        assert_eq!(templates[3]["Recursive"][1]["Inner"][1], "i64");
    }

    // Null values nested in arrays and maps must not be destroyed before their parent
    let nulls =
        deserialize::<Box<dyn MetaCallValue>>(r#"{ "a": [null, "x"], "b": null }"#).unwrap();

    assert!(nulls["a"][0].is::<MetaCallNull>());
    assert_eq!(nulls["a"][1], "x");
    assert!(nulls["b"].is::<MetaCallNull>());
}
