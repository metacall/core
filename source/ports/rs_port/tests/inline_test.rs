use metacall::inline::py;

#[test]
fn test_inline() {
    match metacall::initialize() {
        Err(e) => {
            println!("{}", e);
            panic!();
        }
        _ => println!(" Hello World Metacall created "),
    }

    py! {
        print("hello world")
    }

    metacall::destroy()
}
