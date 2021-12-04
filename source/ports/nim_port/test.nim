import unittest
import metacall

suite "MetaCall Nim Tests":
  test "Load From Memory":
    # Initialize MetaCall
    require(initialize() == 0)

    # Load a script from a buffer
    let tag = "node"
    let script = "module.exports = { add: (a, b) => a + b };"
    check(load_from_memory(tag.cstring, script, len(script).csize_t, nil) == 0)

    # Initialize call arguments
    let args: array[2,pointer] = [
        value_create_double(3.0),
        value_create_double(6.0)
    ]
    let name = "add"

    # Invoke the function add of NodeJS
    var result = v_s(name.cstring, args.unsafeAddr, 2)

    # Clear arguments
    value_destroy(args[0])
    value_destroy(args[1])

    # Check if the result value is correct
    check(value_to_double(result) == 9.0)

    # Clear result value
    value_destroy(result)

    # Destroy MetaCall
    require(destroy() == 0)
