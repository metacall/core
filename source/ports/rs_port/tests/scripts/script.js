async function test_future_resolve() {
  return "hi there!";
}
async function test_future_reject() {
  return Promise.reject("hi there!").catch(() => {});
}
function test_function() {
  return () => "hi there!";
}
function test_exception() {
  return Error("hi there!");
}
function test_throwable() {
  throw new Error("hi there!");
}
function return_the_argument_js(argument) {
  return argument;
}

module.exports = {
  return_the_argument_js,
  test_exception,
  test_function,
  test_future_reject,
  test_future_resolve,
  test_throwable,
};
