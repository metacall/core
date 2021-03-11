// https://gist.githubusercontent.com/MaiaVictor/fdc33c63b9a59b5c093ca35d7ebe44de

var kind = require("kind-lang");

// Parses a Kind file and returns a JSON representation of its defintions.
// Not actually JSON because lambdas are high order, so, for example, `(x) x(K)`
// is represented as:
// {
//   _: "Kind.Term.lam",
//   name: "x",
//   body: x =>
//     {
//       _: "Kind.Term.app",
//       func: x,
//       argm: {_: "Kind.Term.ref", name: "K"}
//     }
// }
function parse_kind_file(code) {
  var file = "Main.kind";
  var parsed = kind["Kind.Defs.read"](file)(code)(kind["Map.new"]);
  switch (parsed._) {
    case "Either.left":
      throw "Parse error.";
    case "Either.right":
      var list = kind["Map.to_list"](parsed.value);
      var defs = {};
      console.log(list)
      while (list._ !== "List.nil") {
        defs[kind["Kind.Name.from_bits"](list.head.fst)] = {
          term: list.head.snd.term,
          type: list.head.snd.type,
        };
        list = list.tail;
      }
      return defs;
  }
};

var defs = parse_kind_file(`
type Bool {
  true
  false
}

not(b: Bool): Bool
  case b {
    true: false
    false: true
  }
`);

// You can also stringify:
console.log(kind["Kind.Term.show"](defs.not.type));
console.log(kind["Kind.Term.show"](defs.not.term));

// You can use other functions to type-check, evaluate, etc.
// Let me know if you have any questions
