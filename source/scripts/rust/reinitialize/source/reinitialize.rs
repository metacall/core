// Script used for verifying that a script without templates can be loaded again after
// reinitializing metacall, even if the library of the previous initialization cannot
// be unloaded because a thread local destructor is still pending

use std::cell::{Cell, RefCell};
use std::sync::atomic::{AtomicI32, Ordering};

// Calls received by this copy of the library, a new copy starts from zero,
// so a library reused from a previous initialization would not start from zero
static CALLS: AtomicI32 = AtomicI32::new(0);

// Thread local destructors run in this copy of the library
static DESTRUCTORS: AtomicI32 = AtomicI32::new(0);

struct Guard;

impl Drop for Guard {
    fn drop(&mut self) {
        DESTRUCTORS.fetch_add(1, Ordering::SeqCst);
    }
}

thread_local! {
    // Thread locals with drop glue register a destructor for the thread that uses them
    static GUARD: RefCell<Option<Guard>> = const { RefCell::new(None) };
    static THREAD_CALLS: Cell<i32> = const { Cell::new(0) };
}

fn call() -> i32 {
    GUARD.with(|guard| {
        let mut guard = guard.borrow_mut();

        if guard.is_none() {
            *guard = Some(Guard);
        }
    });

    THREAD_CALLS.with(|calls| calls.set(calls.get() + 1));

    CALLS.fetch_add(1, Ordering::SeqCst) + 1
}

pub fn calls() -> i32 {
    CALLS.load(Ordering::SeqCst)
}

pub fn thread_calls() -> i32 {
    THREAD_CALLS.with(|calls| calls.get())
}

pub fn destructors() -> i32 {
    DESTRUCTORS.load(Ordering::SeqCst)
}

pub fn bump() -> i32 {
    call()
}

pub fn identity_i32(value: i32) -> i32 {
    call();
    value
}

pub fn identity_f64(value: f64) -> f64 {
    call();
    value
}

pub fn pair_i32_f64(_first: i32, second: f64) -> f64 {
    call();
    second
}
