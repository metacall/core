fn main() {
    match tokio::runtime::Runtime::new()
        .expect("meme")
        .block_on(deno::run_command(
            deno::flags::flags_from_vec(vec![
                "deno".to_string(),
                "run".to_string(),
                "--unstable".to_string(),
                "--allow-plugin".to_string(),
                "test.ts".to_string(),
            ]),
            "test.ts".to_string(),
        )) {
        Ok(_x) => println!("OK"),
        Err(x) => println!("{}", x),
    };
}
