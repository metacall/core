function test_bool_false() {
  return false;
}
function test_bool_true() {
  return true;
}

function test_string() {
  return "Hi there!";
}

function test_map() {
  return { hi: "There", hello: "World" };
}

module.exports = {
  test_bool_false,
  test_bool_true,
  test_string,
  test_map,
};
