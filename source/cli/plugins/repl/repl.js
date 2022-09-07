const repl = require('node:repl');

function repl_main (execute_cb, get_function_list_cb, commands) {
  if (execute_cb == undefined ||
   get_function_list_cb == undefined || 
   commands == undefined)
    throw new Error ("Repl called with invalid arguments");

  const runtime_tags = ["node", "py", "mock", "file", "rb", "cs", "ts", "c", "cob"];
  let dictionary = [...commands];

  //Build completion lists
  for (let tag of runtime_tags) {
    dictionary.push("load " + tag);
    dictionary.push("eval " + tag);
    dictionary.push("clear " + tag);
  }

  for (let cmd of commands) {
    dictionary.push("help " + cmd);
  }

  function build_call_completion_list() {
    const func_list = get_function_list_cb().split("\n");
    let call_completion_list = [];
    for (let func of func_list) {
      call_completion_list.push("call " + func);
    }

    return call_completion_list;
  }

  const splice_idx = dictionary.length;
  function completer (cmd) {
    let hits = [];
    dictionary.splice(splice_idx, dictionary.length - splice_idx, ...build_call_completion_list());
    for (let word of dictionary) {
      if (word.startsWith(cmd))
        hits.push(word);
    }

    return [(hits.length && cmd.length) ? hits : commands, cmd];
  }

  function evaluator (cmd, context, file, callback) {
    if (cmd == "exit") {
      process.exit(0);
    }
    const ret = execute_cb(cmd);
    console.log(ret);
    callback(null);
  }

  //Welcome message
  console.log("Welcome to Tijuana, tequila, sexo & marijuana.");
  
  repl.start({
    prompt: '> ',
    terminal: true,
    eval: evaluator,
    completer: completer,
    preview: true
  });
}

module.exports = repl_main;
