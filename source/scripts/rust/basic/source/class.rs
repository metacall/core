#[repr(C)]
pub struct Book {
    pub price: i32,
}

impl Book {
    pub fn new(price: i32) -> Self {
        Self { price }
    }
    pub fn get_price(&self) -> i32 {
        self.price
    }
    pub fn get_number() -> i32 {
        123
    }
    pub fn null() {}
}

impl Drop for Book {
    fn drop(&mut self) {}
}
pub trait BookTrait {
    fn buy_book(&self, p: i32);
}
impl BookTrait for Book {
    fn buy_book(&self, p: i32) {}
}
