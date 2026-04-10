# MSan Instrumentation Helpers

This folder contains helper scripts and notes for reproducing MemorySanitizer flows with instrumented dependencies.

## Scope

- Build LLVM toolchain artifacts needed by MSan workflows.
- Build libc++/libc++abi with MSan-compatible flags.
- Build gtest with MSan flags and clang toolchain.

## Notes

- Keep this flow separate from default builds until integration is finalized.
- Prefer explicit scripts over ad-hoc command snippets for reproducibility.
