const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const metacall_mod = b.addModule("metacall", .{
        .root_source_file = b.path("src/root.zig"),
        .target = target,
        .optimize = optimize,
    });

    const exe = b.addExecutable(.{
        .name = "integrated_test",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/tests/integrated.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });

    exe.root_module.addImport("metacall", metacall_mod);
    
    // Link libc and libmetacall
    exe.root_module.linkSystemLibrary("c", .{});
    
    // Use environment variables or default build path
    const build_path = b.option([]const u8, "metacall_build_path", "Path to MetaCall build directory") orelse "../../../build";
    
    exe.root_module.addLibraryPath(.{ .cwd_relative = build_path });
    exe.root_module.addIncludePath(b.path("../../metacall/include"));
    exe.root_module.linkSystemLibrary("metacall", .{});

    // Add run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run integrated tests");
    run_step.dependOn(&run_cmd.step);

    // Standard test step
    const unit_tests = b.addTest(.{
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/tests/integrated.zig"),
            .target = target,
            .optimize = optimize,
        }),
    });
    unit_tests.root_module.addImport("metacall", metacall_mod);
    unit_tests.root_module.linkSystemLibrary("c", .{});
    unit_tests.root_module.addLibraryPath(.{ .cwd_relative = build_path });
    unit_tests.root_module.addIncludePath(b.path("../../metacall/include"));
    unit_tests.root_module.linkSystemLibrary("metacall", .{});

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&unit_tests.step);
}
