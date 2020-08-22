struct Defer<F: FnOnce()>(Option<F>);

impl<F: FnOnce()> Drop for Defer<F> {
    fn drop(&mut self) {
        self.0.take().map(|f| f());
    }
}

/// Defer execution of a closure until the constructed value is dropped
/// Works at the end of the scope or manual drop() function
pub fn defer<F: FnOnce()>(f: F) -> impl Drop {
    Defer(Some(f))
}

#[test]
fn test_working() {
    use std::cell::RefCell;

    let i = RefCell::new(0);

    {
        let _d = defer(|| *i.borrow_mut() += 1);
        assert_eq!(*i.borrow(), 0);
    }

    assert_eq!(*i.borrow(), 1);
}