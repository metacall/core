# Instrumentation Utilities

This directory contains local and CI-oriented instrumentation helpers that are intentionally separated from the default build/test paths.

## Goals

- Keep default CI suppression files conservative.
- Keep aggressive debugging suppressions and heavy tooling flows opt-in.
- Provide reproducible scripts for MemorySanitizer and Valgrind investigations.

## Layout

- `python-valgrind/`: build and run helpers for Python with Valgrind support.
- `msan/`: helper scripts for LLVM/libc++/gtest instrumentation workflows.
- `suppressions/`: aggressive suppressions used only for local deep triage.

These files are not automatically consumed by default test jobs unless explicitly wired.
