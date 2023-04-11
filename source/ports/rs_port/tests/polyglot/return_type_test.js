function test_bool(bool) {
  return bool;
}

function test_string(string) {
  return string;
}

function test_map(map) {
  return map;
}

function test_array(array) {
  return ["hi", "there!"];
}

async function test_future_resolve() {
  return "hi there!";
}
async function test_future_reject() {
  return Promise.reject("hi there!").catch(() => {});
}

function test_function(function_a) {
  return function_a;
}

function test_null(null_a) {
  return null_a;
}

function test_exception() {
  return Error("hi there!");
}

function test_throwable() {
  throw new Error("hi there!");
}

module.exports = {
  test_array,
  test_bool,
  test_exception,
  test_function,
  test_future_reject,
  test_future_resolve,
  test_map,
  test_null,
  test_string,
  test_throwable,
};
