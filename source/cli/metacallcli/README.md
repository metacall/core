# MetaCall CLI

## About

MetaCall provides a Command Line Interface which supports all MetaCall functionalities such as loading, calling, awaiting scripts.

Use the `metacall` command in your shell after [installing](https://github.com/metacall/install) MetaCall to launch the CLI.

## Currently Available Commands

- `help`: Show help for MetaCall CLI (**Use this command to see the usage of a particular command**)
- `load`: Load a script from file into MetaCall
- `inspect`: Show all runtimes, modules and functions (with their signature) loaded into MetaCall
- `call`: Call a function previously loaded in MetaCall (Note: For `js` and `py` functions that do not return anything, the default return value is a `null`)
- `await`: Await an async function previously loaded in MetaCall
- `clear`: Delete a script previously loaded in MetaCall
- `exit`: Exit from MetaCall CLI
