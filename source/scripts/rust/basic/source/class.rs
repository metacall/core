#[repr(C)]
struct Book {
    price: i32,
}

impl Book {
    fn new(price: i32) -> Self {
        Self { price }
    }
    fn get_price(&self) -> i32 {
        self.price
    }
    fn get_number() -> i32 {
        123
    }
    fn null() {}
}

impl Drop for Book {
    fn drop(&mut self) {}
}
trait BookTrait {
    fn buy_book(&self, p: i32);
}
impl BookTrait for Book {
    fn buy_book(&self, p: i32) {}
}
