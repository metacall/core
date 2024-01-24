const std = @import("std");
const mb = @import("metacall-bindings.zig");

const Metacall = @This();

/// Initializes MetaCall.
pub fn init() !void {
    if (mb.metacall_initialize() != 0)
        return error.FailedToInitMetacall;
}
/// Deinitializes MetaCall and returns an error if didn't succeed.
pub fn destroy() !void {
    if (mb.metacall_destroy() != 0)
        return error.FailedToDeinitMetacall;
}
/// Deinitializes MetaCall.
pub fn deinit() void {
    _ = destroy() catch {};
}

/// Loads files into MetaCall, strings should be null-terminated.
pub fn load_from_file(tag: [:0]const u8, paths: [][:0]const u8) !void {
    if (mb.metacall_load_from_file(tag.ptr, @ptrCast(paths.ptr), paths.len, null) != 0) {
        return error.FailedToLoadFromFile;
    }
}
/// Loads a string into MetaCall, the `tag` should be null-terminated and the `buffer` should be a buffer.
pub fn load_from_memory(tag: [:0]const u8, buffer: []const u8) !void {
    if (mb.metacall_load_from_memory(tag.ptr, buffer.ptr, buffer.len, null) != 0) {
        return error.FailedToLoadFromMemory;
    }
}
/// Loads a package into MetaCall, the `tag` should be null-terminated and the `path` should be a buffer.
pub fn load_from_package(tag: [:0]const u8, path: []const u8) !void {
    if (mb.metacall_load_from_package(tag.ptr, path.ptr, path.len, null) != 0) {
        return error.FailedToLoadFromPackage;
    }
}

fn parse_arg(value: anytype) ?*anyopaque {
    return switch (@TypeOf(value)) {
        bool => mb.metacall_value_create_bool(@intCast(@intFromBool(value))),
        u8, i8 => mb.metacall_value_create_char(@intCast(value)),
        u16, i16 => mb.metacall_value_create_short(@intCast(value)),
        u32, i32 => mb.metacall_value_create_int(@intCast(value)),
        u64, i64 => mb.metacall_value_create_long(@intCast(value)),
        f32 => mb.metacall_value_create_float(@floatCast(value)),
        f64 => mb.metacall_value_create_double(@floatCast(value)),
        []const u8 => mb.metacall_value_create_string(@ptrCast(value.ptr), value.len),
        [:0]const u8 => mb.metacall_value_create_buffer(@ptrCast(value.ptr), value.len),
        else => mb.metacall_value_create_null(),
    };
}
fn parse_ret(comptime R: type, value: ?*anyopaque) ?R {
    if (value == null) return null;

    const val_id = mb.metacall_value_id(value);
    switch (R) {
        bool => {
            if (val_id == 0)
                return mb.metacall_value_to_bool(value) != 0;
        },
        u8, i8 => {
            if (val_id == 1)
                return mb.metacall_value_to_char(value);
        },
        u16, i16 => {
            if (val_id == 2)
                return @intCast(mb.metacall_value_to_short(value));
        },
        u32, i32 => {
            if (val_id == 3)
                return @intCast(mb.metacall_value_to_int(value));
        },
        u64, i64 => {
            if (val_id == 4)
                return @intCast(mb.metacall_value_to_long(value));
        },
        f32 => {
            if (val_id == 5)
                return @floatCast(mb.metacall_value_to_float(value));
        },
        f64 => {
            if (val_id == 6)
                return @floatCast(mb.metacall_value_to_double(value));
        },
        [*:0]u8 => {
            if (val_id == 7)
                return @ptrCast(mb.metacall_value_to_string(value));
        },
        else => {},
    }

    return null;
}

/// Calls a function with the return type of `R`. The `args` argument should be an array of any Metacall supported type.
pub fn metacall(comptime R: type, method: [:0]const u8, args: anytype) ?R {
    const info = @typeInfo(@TypeOf(args));
    comptime {
        if (info != .Array)
            @compileError("Arguments should be an array!");
    }
    var metacall_args: [args.len]?*anyopaque = undefined;

    for (args, 0..) |arg, idx| {
        metacall_args[idx] = parse_arg(arg);
    }

    const metacall_ret = mb.metacallv_s(method, &metacall_args, metacall_args.len);
    defer {
        for (metacall_args) |arg| mb.metacall_value_destroy(arg);
        mb.metacall_value_destroy(metacall_ret);
    }

    return parse_ret(R, metacall_ret);
}
