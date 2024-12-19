const std = @import("std");
const metacall = @import("metacall");

const hi: []const u8 = "hi";

pub fn main() !void {
    try metacall.init();

    var paths: [1][:0]const u8 = .{"/home/raymond/Projects/metacall-core/source/ports/zig_port/src/tests/test.c"};
    try metacall.load_from_file("c", &paths);

    var paths_py: [1][:0]const u8 = .{"/home/raymond/Projects/metacall-core/source/ports/zig_port/src/tests/test.py"};
    try metacall.load_from_file("py", &paths_py);

    const ret_bool = metacall.metacall(bool, "ret_bool", [0]bool{});
    try std.testing.expect(ret_bool == true);

    const ret_char = metacall.metacall(u8, "sum_char", [2]u8{ 1, 1 });
    try std.testing.expect(ret_char == 2);

    const ret_short = metacall.metacall(u16, "sum_short", [2]u8{ 1, 1 });
    try std.testing.expect(ret_short == 2);

    const ret_int = metacall.metacall(u32, "sum_int", [2]u32{ 1, 1 });
    try std.testing.expect(ret_int == 2);

    const ret_long = metacall.metacall(u64, "sum_long", [2]u64{ 1, 1 });
    try std.testing.expect(ret_long == 2);

    const ret_float = metacall.metacall(f32, "sum_float", [2]f32{ 1.0, 1.0 });
    try std.testing.expect(ret_float == 2.0);

    const ret_double = metacall.metacall(f64, "sum_double", [2]f64{ 1.0, 1.0 });
    try std.testing.expect(ret_double == 2.0);

    const ret_string = metacall.metacall([*:0]u8, "ret_string", [1][]const u8{hi});
    try std.testing.expect(ret_string != null);
    try std.testing.expect(ret_string.?[0] == 'h');
    try std.testing.expect(ret_string.?[1] == 'i');

    defer metacall.destroy();
}
