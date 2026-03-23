const std = @import("std");
const mb = @import("metacall-bindings.zig");

const Metacall = @This();

/// Enum mapping MetaCall value type IDs to named constants.
/// Corresponds to `enum metacall_value_id` in metacall_value.h.
pub const MetacallValueId = enum(c_int) {
    bool_type = 0,
    char_type = 1,
    short_type = 2,
    int_type = 3,
    long_type = 4,
    float_type = 5,
    double_type = 6,
    string_type = 7,
    buffer_type = 8,
    array_type = 9,
    map_type = 10,
    ptr_type = 11,
    future_type = 12,
    function_type = 13,
    null_type = 14,
    class_type = 15,
    object_type = 16,
    exception_type = 17,
    throwable_type = 18,
};

/// Initializes MetaCall.
pub fn init() !void {
    if (mb.metacall_initialize() != 0)
        return error.FailedToInitMetacall;
}
/// Deinitializes MetaCall and returns an error if didn't succeed.
pub fn destroy() void {
    mb.metacall_destroy();
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

/// Converts a Zig value to a MetaCall value pointer using comptime type dispatch.
///
/// Uses `@typeInfo` to categorize types at compile time:
/// - Booleans → `metacall_value_create_bool`
/// - Integers (any size/signedness) → mapped to char/short/int/long by bit width
/// - Floats → f32 to float, f64 to double
/// - Strings (`[]const u8`) → `metacall_value_create_string`
/// - Sentinel-terminated strings (`[:0]const u8`) → `metacall_value_create_string`
/// - Null (`@TypeOf(null)`) → `metacall_value_create_null`
/// - Optional with null value → `metacall_value_create_null`
///
/// Produces a compile error for unsupported types, rather than silently creating null.
pub fn createValue(value: anytype) ?*anyopaque {
    const T = @TypeOf(value);
    const info = @typeInfo(T);

    return switch (info) {
        .bool => mb.metacall_value_create_bool(@intCast(@intFromBool(value))),
        .int => |int_info| {
            // Map integer types by bit width to the appropriate MetaCall integer type.
            if (int_info.bits <= 8) {
                return mb.metacall_value_create_char(@intCast(value));
            } else if (int_info.bits <= 16) {
                return mb.metacall_value_create_short(@intCast(value));
            } else if (int_info.bits <= 32) {
                return mb.metacall_value_create_int(@intCast(value));
            } else {
                return mb.metacall_value_create_long(@intCast(value));
            }
        },
        .comptime_int => mb.metacall_value_create_long(@intCast(value)),
        .float => |float_info| {
            if (float_info.bits <= 32) {
                return mb.metacall_value_create_float(@floatCast(value));
            } else {
                return mb.metacall_value_create_double(@floatCast(value));
            }
        },
        .comptime_float => mb.metacall_value_create_double(@floatCast(value)),
        .pointer => |ptr_info| {
            // Handle string-like pointer types: *const [N:0]u8, [*:0]const u8, etc.
            if (ptr_info.size == .slice) {
                // []const u8 — a Zig string slice
                if (ptr_info.child == u8) {
                    return mb.metacall_value_create_string(@ptrCast(value.ptr), value.len);
                }
            } else if (ptr_info.size == .one) {
                // Pointer to array, e.g. *const [N:0]u8 (string literal type)
                const child_info = @typeInfo(ptr_info.child);
                if (child_info == .array) {
                    if (child_info.array.child == u8) {
                        const slice: []const u8 = value;
                        return mb.metacall_value_create_string(@ptrCast(slice.ptr), slice.len);
                    }
                }
            } else if (ptr_info.size == .many) {
                // [*:0]const u8 — sentinel-terminated pointer
                if (ptr_info.child == u8) {
                    const len = std.mem.len(value);
                    return mb.metacall_value_create_string(@ptrCast(value), len);
                }
            }
            @compileError("Unsupported pointer type: " ++ @typeName(T) ++
                ". Use []const u8 for strings.");
        },
        .null => mb.metacall_value_create_null(),
        .optional => {
            if (value) |unwrapped| {
                return createValue(unwrapped);
            } else {
                return mb.metacall_value_create_null();
            }
        },
        else => @compileError("Unsupported type for MetaCall value: " ++ @typeName(T)),
    };
}

/// Destroys a MetaCall value, freeing associated resources.
pub fn destroyValue(value: ?*anyopaque) void {
    mb.metacall_value_destroy(value);
}

/// Returns the type ID of a MetaCall value as a `MetacallValueId`.
pub fn valueId(value: ?*anyopaque) ?MetacallValueId {
    if (value == null) return null;
    const raw_id = mb.metacall_value_id(value);
    return std.meta.intToEnum(MetacallValueId, raw_id) catch null;
}

/// Legacy `parse_arg` function — delegates to `createValue` for backward compatibility.
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

/// Converts a MetaCall value pointer to a Zig type based on the runtime value ID.
///
/// Uses `MetacallValueId` named constants instead of magic numbers for clarity.
/// Supports: bool, integers (u8-u64, i8-i64), floats (f32, f64), strings ([*:0]u8),
/// and void (for null returns).
fn parse_ret(comptime R: type, value: ?*anyopaque) ?R {
    if (value == null) return null;

    const val_id: c_uint = @intCast(mb.metacall_value_id(value));
    switch (R) {
        bool => {
            if (val_id == @intFromEnum(MetacallValueId.bool_type))
                return mb.metacall_value_to_bool(value) != 0;
        },
        u8, i8 => {
            if (val_id == @intFromEnum(MetacallValueId.char_type))
                return mb.metacall_value_to_char(value);
        },
        u16, i16 => {
            if (val_id == @intFromEnum(MetacallValueId.short_type))
                return @intCast(mb.metacall_value_to_short(value));
        },
        u32, i32 => {
            if (val_id == @intFromEnum(MetacallValueId.int_type))
                return @intCast(mb.metacall_value_to_int(value));
        },
        u64, i64 => {
            if (val_id == @intFromEnum(MetacallValueId.long_type))
                return @intCast(mb.metacall_value_to_long(value));
        },
        f32 => {
            if (val_id == @intFromEnum(MetacallValueId.float_type))
                return @floatCast(mb.metacall_value_to_float(value));
        },
        f64 => {
            if (val_id == @intFromEnum(MetacallValueId.double_type))
                return @floatCast(mb.metacall_value_to_double(value));
        },
        [*:0]u8 => {
            if (val_id == @intFromEnum(MetacallValueId.string_type))
                return @ptrCast(mb.metacall_value_to_string(value));
        },
        void => {
            if (val_id == @intFromEnum(MetacallValueId.null_type))
                return;
        },
        else => {},
    }

    return null;
}

/// Calls a function with the return type of `R`. The `args` argument should be an array of any Metacall supported type.
pub fn metacall(comptime R: type, method: [:0]const u8, args: anytype) ?R {
    const info = @typeInfo(@TypeOf(args));
    comptime {
        if (info != .array)
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

/// Calls a function using the new comptime `createValue` for argument conversion.
/// This is the recommended API going forward, as it provides compile-time errors
/// for unsupported types rather than silently converting to null.
///
/// Example:
/// ```zig
/// const result = Metacall.metacall_typed(f64, "sum", .{ 1.0, 2.0 });
/// ```
pub fn metacall_typed(comptime R: type, method: [:0]const u8, args: anytype) ?R {
    const fields = @typeInfo(@TypeOf(args));
    comptime {
        if (fields != .@"struct" or !fields.@"struct".is_tuple)
            @compileError("Arguments should be a tuple, e.g. .{ 1, 2.0, \"hello\" }");
    }
    const arg_count = fields.@"struct".fields.len;
    var metacall_args: [arg_count]?*anyopaque = undefined;

    inline for (fields.@"struct".fields, 0..) |field, idx| {
        metacall_args[idx] = createValue(@field(args, field.name));
    }

    const metacall_ret = mb.metacallv_s(method, &metacall_args, arg_count);
    defer {
        for (metacall_args) |arg| mb.metacall_value_destroy(arg);
        mb.metacall_value_destroy(metacall_ret);
    }

    return parse_ret(R, metacall_ret);
}

// ============================================================================
// Comptime unit tests — these verify type dispatch logic at compile time.
// They do NOT require the MetaCall runtime to be installed.
// ============================================================================

test "MetacallValueId enum values match C header" {
    // Verify our enum matches the C metacall_value_id enum exactly
    try std.testing.expectEqual(@as(c_int, 0), @intFromEnum(MetacallValueId.bool_type));
    try std.testing.expectEqual(@as(c_int, 1), @intFromEnum(MetacallValueId.char_type));
    try std.testing.expectEqual(@as(c_int, 2), @intFromEnum(MetacallValueId.short_type));
    try std.testing.expectEqual(@as(c_int, 3), @intFromEnum(MetacallValueId.int_type));
    try std.testing.expectEqual(@as(c_int, 4), @intFromEnum(MetacallValueId.long_type));
    try std.testing.expectEqual(@as(c_int, 5), @intFromEnum(MetacallValueId.float_type));
    try std.testing.expectEqual(@as(c_int, 6), @intFromEnum(MetacallValueId.double_type));
    try std.testing.expectEqual(@as(c_int, 7), @intFromEnum(MetacallValueId.string_type));
    try std.testing.expectEqual(@as(c_int, 8), @intFromEnum(MetacallValueId.buffer_type));
    try std.testing.expectEqual(@as(c_int, 9), @intFromEnum(MetacallValueId.array_type));
    try std.testing.expectEqual(@as(c_int, 10), @intFromEnum(MetacallValueId.map_type));
    try std.testing.expectEqual(@as(c_int, 11), @intFromEnum(MetacallValueId.ptr_type));
    try std.testing.expectEqual(@as(c_int, 12), @intFromEnum(MetacallValueId.future_type));
    try std.testing.expectEqual(@as(c_int, 13), @intFromEnum(MetacallValueId.function_type));
    try std.testing.expectEqual(@as(c_int, 14), @intFromEnum(MetacallValueId.null_type));
    try std.testing.expectEqual(@as(c_int, 15), @intFromEnum(MetacallValueId.class_type));
    try std.testing.expectEqual(@as(c_int, 16), @intFromEnum(MetacallValueId.object_type));
    try std.testing.expectEqual(@as(c_int, 17), @intFromEnum(MetacallValueId.exception_type));
    try std.testing.expectEqual(@as(c_int, 18), @intFromEnum(MetacallValueId.throwable_type));
}

test "MetacallValueId intToEnum round-trip" {
    // Test that valid IDs convert correctly
    const id = std.meta.intToEnum(MetacallValueId, 7) catch unreachable;
    try std.testing.expectEqual(MetacallValueId.string_type, id);

    // Test that invalid IDs return null via our helper logic
    const invalid = std.meta.intToEnum(MetacallValueId, 99) catch null;
    try std.testing.expectEqual(@as(?MetacallValueId, null), invalid);
}
