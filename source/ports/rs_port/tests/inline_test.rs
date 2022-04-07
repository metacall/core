use metacall::metacall_inline::{py, node, rb, ts};

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

    node! {
        console.log("hello world");
    }

    rb! {
        print "hello world\n"
    }

    ts! {
        console.log("hello world");
    }

    metacall::destroy()
}
