# MetaCall Zig Port

Zig language bindings for the [MetaCall](https://github.com/metacall/core) polyglot runtime. Call functions across programming languages (Python, JavaScript, TypeScript, Ruby, C, and more) from idiomatic Zig, with comptime-based type-safe APIs.

## Features

- **Comptime type dispatch** — Zig types are automatically mapped to MetaCall value types at compile time using `@typeInfo`
- **Type-safe API** — `createValue` produces compile errors for unsupported types instead of silently creating null values
- **Named value IDs** — `MetacallValueId` enum replaces magic numbers, matching the C `metacall_value_id` enum exactly
- **Multiple call styles** — Legacy array-based `metacall()` and new tuple-based `metacall_typed()` for ergonomic usage
- **Error unions** — Load functions return Zig error unions for safe error handling

## Prerequisites

- [Zig](https://ziglang.org/download/) 0.12.0 or later
- [MetaCall](https://github.com/metacall/install) runtime installed

## Quick Start

```zig
const metacall = @import("metacall");

pub fn main() !void {
    // Initialize MetaCall
    try metacall.init();
    defer metacall.destroy();

    // Load a TypeScript file
    var paths: [1][:0]const u8 = .{"sum.ts"};
    try metacall.load_from_file("ts", &paths);

    // Call a function (legacy array style)
    const result = metacall.metacall(f64, "sum", [2]f64{ 1.0, 2.0 });
    // result == 3.0

    // Call a function (new tuple style — recommended)
    const result2 = metacall.metacall_typed(f64, "sum", .{ 1.0, 2.0 });
    // result2 == 3.0
}
```

## API Reference

| Function | Description |
|---|---|
| `init() !void` | Initialize the MetaCall runtime |
| `destroy() void` | Deinitialize MetaCall |
| `load_from_file(tag, paths) !void` | Load scripts from file paths |
| `load_from_memory(tag, buffer) !void` | Load a script from a string buffer |
| `load_from_package(tag, path) !void` | Load a script package |
| `metacall(R, name, args) ?R` | Call a function (array args) |
| `metacall_typed(R, name, args) ?R` | Call a function (tuple args, with comptime type safety) |
| `createValue(value) ?*anyopaque` | Convert a Zig value to a MetaCall value (comptime type-safe) |
| `destroyValue(value) void` | Free a MetaCall value |
| `valueId(value) ?MetacallValueId` | Get the type ID of a MetaCall value |

## Supported Type Mappings

| Zig Type | MetaCall Type | Value ID |
|---|---|---|
| `bool` | Boolean | 0 |
| `u8`, `i8` | Char | 1 |
| `u16`, `i16` | Short | 2 |
| `u32`, `i32` | Int | 3 |
| `u64`, `i64` | Long | 4 |
| `f32` | Float | 5 |
| `f64` | Double | 6 |
| `[]const u8` | String | 7 |
| `null` | Null | 14 |

## Building

```sh
# Build the library
zig build

# Run unit tests
zig build test

# With custom MetaCall path
METACALL_PATH=/usr/local zig build
```

## Related

- [MetaCall Core](https://github.com/metacall/core) — The polyglot runtime
- [Rust Port](https://github.com/metacall/core/tree/develop/source/ports/rs_port) — Design reference
- [MetaCall Install](https://github.com/metacall/install) — Installation guide
- [GSoC 2025 Zig Port project](https://github.com/nicolestandifer3/metacall-GSoC-2025) — GSoC project description

## License

Apache License 2.0 — see the [MetaCall Core license](https://github.com/metacall/core/blob/develop/LICENSE).
