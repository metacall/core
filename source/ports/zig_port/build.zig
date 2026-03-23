const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // -- Locate MetaCall headers and library --
    // The MetaCall C library must be installed on the system.
    // Set METACALL_PATH environment variable to the MetaCall install directory,
    // or the build will attempt to find it in system library paths.
    const metacall_path = std.process.getEnvVarOwned(b.allocator, "METACALL_PATH") catch null;
    defer if (metacall_path) |p| b.allocator.free(p);

    // -- Root module (importable by other Zig projects via 'metacall' import) --
    const root_mod = b.createModule(.{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    // -- Unit tests --
    // Comptime tests verify enum values and type dispatch logic without the MetaCall runtime.
    // Integration tests call into libmetacall and require it to be installed.
    const unit_tests = b.addTest(.{
        .root_module = root_mod,
    });
    unit_tests.linkLibC();
    unit_tests.linkSystemLibrary("metacall");
    if (metacall_path) |path| {
        const lib_path = std.fmt.allocPrint(b.allocator, "{s}/lib", .{path}) catch @panic("OOM");
        const inc_path = std.fmt.allocPrint(b.allocator, "{s}/include", .{path}) catch @panic("OOM");
        unit_tests.addLibraryPath(.{ .cwd_relative = lib_path });
        unit_tests.addIncludePath(.{ .cwd_relative = inc_path });
    }

    const run_unit_tests = b.addRunArtifact(unit_tests);
    const test_step = b.step("test", "Run all tests (requires MetaCall runtime installed)");
    test_step.dependOn(&run_unit_tests.step);
}
