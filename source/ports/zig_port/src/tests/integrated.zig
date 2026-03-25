const std = @import("std");
const metacall = @import("metacall");

const hi: []const u8 = "hi";

pub fn main() !void {
    try metacall.init();

    // Use relative paths for better portability
    var paths: [1][:0]const u8 = .{"src/tests/test.c"};
    try metacall.load_from_file("c", &paths);

    var paths_py: [1][:0]const u8 = .{"src/tests/test.py"};
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
    defer ret_string.deinit();
    
    const str = ret_string.get();
    try std.testing.expect(str != null);
    try std.testing.expect(str.?[0] == 'h');
    try std.testing.expect(str.?[1] == 'i');

    // Stress test: Call 10,000 times to verify memory stability
    var i: usize = 0;
    while (i < 10000) : (i += 1) {
        const stress_ret = metacall.metacall([*:0]u8, "ret_string", [1][]const u8{hi});
        defer stress_ret.deinit();
        try std.testing.expect(stress_ret.get() != null);
    }

    // Edge case: Empty string
    const empty_str_ret = metacall.metacall([*:0]u8, "ret_string", [1][]const u8{""});
    defer empty_str_ret.deinit();
    try std.testing.expect(empty_str_ret.get() != null);
    try std.testing.expect(empty_str_ret.get().?[0] == 0);

    defer metacall.destroy();
}
