use metacall::inline::py;

fn main() {
    py! {
        print("hello world")
    }
}
