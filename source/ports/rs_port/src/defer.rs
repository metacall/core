/*
 *	MetaCall Library by Parra Studios
 *	A library for providing a foreign function interface calls.
 *
 *	Copyright (C) 2016 - 2020 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

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
fn test() {
	use std::cell::RefCell;

	let i = RefCell::new(0);

	{
		let _d = defer(|| *i.borrow_mut() += 1);
		assert_eq!(*i.borrow(), 0);
	}

	assert_eq!(*i.borrow(), 1);
}
