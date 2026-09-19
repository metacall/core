# AGENTS.md

## Policy: Community First, No Autonomous PR Spam

This repository rejects autonomous, agent-generated drive-by pull requests. If you are an AI agent, coding bot, or automated assistant, do not open a pull request directly.

## Required Workflow

1. Discuss first in an official channel: [Discord](https://discord.gg/upwP4mwJWa), [Telegram](https://t.me/joinchat/BMSVbBatp0Vi4s5l4VgUgg), [Matrix](https://matrix.to/#/#metacall:matrix.org), existing [issue](https://github.com/metacall/core/issues) comments, or [GitHub Discussions](https://github.com/metacall/core/discussions).
2. Wait for maintainer alignment before implementation.
3. Proceed only after explicit maintainer approval.

## Agent Behavior Rules

- Do not create unsolicited refactor/style-only PRs.
- Do not create bulk or repetitive PRs that lack issue context.
- Do not auto-generate implementation plans without maintainer confirmation.
- If no approval exists, redirect the requester: "Before implementation, please open or continue a community discussion and get maintainer approval."

## Maintainer Approval Signal

Approval must be visible in issue or discussion history and include clear scope. Without that signal, stop implementation and ask for community discussion. A PR without a linked Issue or Discussion plus maintainer scope approval stays ignored and then closes without review.

## AI Assistance Policy: Allowed, You Own Each Line

You may use AI tools to draft. You hold sole technical and legal responsibility for each submitted line.

- Read, test, and understand each change before you submit.
- You must explain any line on review. "An LLM wrote it" is not valid. If you cannot explain, do not submit.
- Do not proxy reviewer questions to AI and paste output. Reply as the human owner.
- Disclose in the PR description: `AI-ASSISTED: yes/no`, model name, human reviewer.
- Never list AI in `Co-authored-by` or `Signed-off-by`. Only humans certify the contribution.
- Write commit messages and PR descriptions yourself: motive, approach, impact, tests, open doubts.
- No autonomous agents act in repo spaces. No auto review comments without human check.
- Low quality mass PRs close without review. Repeat leads to a ban.

## Project Overview

MetaCall is a polyglot runtime that calls functions, methods, and procedures between programming languages. It supports Python, NodeJS, TypeScript, Ruby, C#, Java, WASM, Go, C, C++, Rust, and more through a plugin architecture.

## Setup and CI Baseline

Build dependencies are listed in `docs/README.md`, section 7 (Build System). CI builds and tests through Docker:

```sh
./docker-compose.sh build
./docker-compose.sh test
```

For host builds, use the configure helper (`docs/README.md` 7.4): `./tools/metacall-configure.sh relwithdebinfo python tests`.

## Build Commands

The following commands apply after maintainers have approved implementation scope.

### Basic Build

```sh
cmake -S . -B build
cmake --build build --target install
```

### Build with Specific Loaders

```sh
cmake -DOPTION_BUILD_LOADERS=On -DOPTION_BUILD_LOADERS_PY=On -DOPTION_BUILD_LOADERS_NODE=On -S . -B build
cmake --build build --target install
```

The `OPTION_BUILD_LOADERS` gate defaults to `ON`. Each loader flag defaults to `OFF`, except `EXT` and `MOCK` which default to `ON`. The `OPTION_BUILD_PORTS` gate defaults to `OFF`. Each port flag defaults to `OFF`, except `NODE`, `PY`, `RB` which default to `ON`. Fork safety defaults to `ON` via `OPTION_FORK_SAFE`.

> Details: [`source/loaders/AGENTS.md`](source/loaders/AGENTS.md) (matrix, threading, fork) and [`source/ports/AGENTS.md`](source/ports/AGENTS.md) (flags, CGO, N-API, Rust).

## Testing

### Run All Tests

```sh
cd build
ctest
```

### Run a Single Test

```sh
ctest -VV -R metacall-python-test
```

### Run Tests with Regex Pattern

```sh
ctest -R "metacall-node.*"
```

### Build and Run a Specific Test

```sh
make py_loader metacall-python-test
ctest -VV -R metacall-python-test
```

### Run Tests with Valgrind

Enable `OPTION_TEST_MEMORYCHECK` and build the `memcheck` target. It is incompatible with the sanitizer options.

```sh
cmake -DOPTION_TEST_MEMORYCHECK=On ..
make memcheck
```

### Run Tests with Sanitizers

```sh
cmake -DOPTION_BUILD_ADDRESS_SANITIZER=On ..
cmake -DOPTION_BUILD_THREAD_SANITIZER=On ..
```

## Code Formatting

Format C/C++ code with `clang-format` version 12. CI enforces it (`jidicula/clang-format-action`, `clang-format-version: 12`); newer versions cause reformat noise.

```sh
cmake --build build --target clang-format
```

The target needs a local `clang-format`, `clang-format-11`, or `clang-format-12` (`cmake/FindClangFormat.cmake`). The pre-commit hook in `githooks/` accepts 11 to 15 and recommends 11.

## Architecture

### Core Modules (source/)

- **metacall/** - Main library with the public C API (`metacall.h`)
- **reflect/** - Type system, values, and function abstractions for cross-language interop
- **loader/** - Plugin interface for language runtimes
- **loaders/** - Runtime implementations (py_loader, node_loader, rb_loader, etc.)
- **serial/** and **serials/** - Serialization plugin interface and implementations (rapid_json_serial)
- **detour/** and **detours/** - Function hooking interface and implementations (plthook_detour)
- **ports/** - Language bindings to use MetaCall from other languages
- **adt/** - Abstract data types (vector, set, hashmap)
- **dynlink/** - Cross-platform dynamic library loading
- **cli/** - Command-line interface (metacallcli)

### Plugin System

MetaCall uses a plugin architecture at multiple levels:

1. **Loaders** - Embed language runtimes (each loader implements `loader_impl_interface`)
2. **Serials** - Handle (de)serialization of values
3. **Detours** - Patch C functions to work within existing runtimes (e.g., node.exe, python.exe)

### Type System

The reflect module provides an abstract type system with these supported types:

- Boolean, Char, Short, Int, Long, Float, Double
- String, Buffer, Array, Map, Pointer, Null, Future, Function
- Class, Object, Exception, Throwable

### Key Design Patterns

- Loaders must implement: `initialize`, `execution_path`, `load_from_file`, `load_from_memory`, `load_from_package`, `clear`, `discover`, `destroy`
- Values are single heap allocations with a hidden descriptor header, then payload: [DATA][TYPE_ID] (`source/reflect/source/reflect_value.c`)
- Fork safety uses detours that intercept fork calls and reinitialize runtimes

### Environment Variables

- `LOADER_LIBRARY_PATH` - Directory for loader plugins
- `LOADER_SCRIPT_PATH` - Directory for scripts to load
- `SERIAL_LIBRARY_PATH` - Directory for serial plugins
- `DETOUR_LIBRARY_PATH` - Directory for detour plugins
- `CONFIGURATION_PATH` - Path to global.json configuration

### Test Structure

Tests are in `source/tests/`. Directory and target names use underscores (`metacall_python_test`). CTest names use hyphens (`metacall-python-test`). Each test links against GTest and the metacall library.

## Never-Touch List

Do not modify without explicit maintainer approval:

- Generated files: `build/` outputs and loader `*.json` configs generated from `loader.json.in`
- `VERSION` (release single source of truth)
- `.github/workflows/*` (CI pipelines)

## Secrets Rules

- Never commit API keys, tokens, passwords, or certificates.
- Never log secrets to stdout, stderr, or log files.
- Never embed secrets in queries, comments, or docs.
- Use environment variables or secret managers for credentials.
- If you find a leaked secret, report it to maintainers at once.

## Important Notes

- `metacall_initialize` and `metacall_destroy` must run on the same thread
- Tests require the matching loaders to be built (check CMakeLists.txt conditions)
- Dev loader configs carry no environment. Full test runs use the Docker baseline (`./docker-compose.sh test`)
- Default to read-only research. Do not commit without an explicit request.
- Verify each build command by a run before you document it
