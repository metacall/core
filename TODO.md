# MetaCall Roadmap

> High-level architectural roadmap, feature milestones, and backlog triage for the MetaCall polyglot runtime.

---

## Horizon Overview

| Status Icon | Meaning |
| :---: | :--- |
| :white_check_mark: | **Done** (Implemented / Shipped) |
| :hammer_and_wrench: | **In Progress** (Active implementation) |
| :calendar: | **Planned** (Scoped for implementation) |
| :pause_button: | **Deferred** (Postponed / Awaiting prerequisites) |
| :x: | **Dropped** (Deprecated / Retired) |

---

## Now (0 - 3 Months)

### Core & Reflect
* :hammer_and_wrench: **Thread Safety:** Enforce comprehensive thread safety across core boundaries and plugin invocations ([#863](https://github.com/metacall/core/issues/863)).
* :hammer_and_wrench: **Unified Error Management:** Implement unified error propagation across runtimes.
* :white_check_mark: **Community Health & Standards:** Standardize contributing guidelines, issue workflows, and code style rules (Project Rosetta-4).
* :calendar: **Scope & Lifetimes:** Improve scope encapsulation and resolve function name collision tie-breaking (no issue yet).
* :calendar: **Sanitizers & Tooling:** Integrate ASan, TSan, and strict compiler flags (`-Wall -Wextra -Werror`) across CMake builds.

### Documentation (Project Rosetta-1)
* :hammer_and_wrench: **Automated Docs Publishing:** Modernize docs infrastructure and set up automated static documentation generation (Rosetta-1).
* :white_check_mark: **Governance & Security Baseline:** Publish root `SECURITY.md` and `ROADMAP.md` (Rosetta-5, Rosetta-6).

---

## Next (3 - 12 Months)

### Loaders & Ports (Project Rosetta-3)
* :calendar: **Loader Stability Tiers:** Establish and promote loaders across defined stability tiers (Rosetta-3).
* :calendar: **Loader Improvements & Backlog:**
  - **C/C++ Loader:** Dynamic linking and JIT integration via Clang/LLVM and libffi.
  - **C# Loader:** Upgraded hosting support for modern .NET Core runtimes.
  - **Java / JVM:** JNI runtime integration and lifecycle management.
  - **Scripting Loaders:** Modernize and harden PHP and Lua C API embed integrations.
* :calendar: **SWIG Ports Overhaul:** Modernize automated language port generation and eliminate legacy binding layers.

### Core Architecture
* :calendar: **Generic Plugin System:** Merge serial, detour, and loader architectures into a unified, versioned plugin system.
* :calendar: **Extended Script Loading:** Support script loading directly from strings, URIs, and raw memory blocks.
* :calendar: **Dynamic Watchers:** Implement file watchers and live reloading with fault tolerance against corrupted scripts.
* :calendar: **Type Management Extensions:** Expand core type management and loader mapping for pointers, arrays, callbacks, and composite objects.
* :calendar: **Automated Test Architecture:** Standardize TDD testing workflows and unified test harness across loaders.
* :calendar: **Cross-Platform Path Management:** Add cross-platform path tree management for script discovery.

### Build, CI & Deployment
* :calendar: **Automated Packaging:** CI/CD pipelines for automated release of native `.deb` and `.rpm` packages.
* :calendar: **Cross-Compilation:** CMake-integrated cross-compiling toolchains for target architectures.

---

## Later (12+ Months)

### MetaCall++ & Advanced Core
* :calendar: **MetaCall++ Front-End:** Native modern C++ front-end with SWIG compatibility.
* :calendar: **Meta-Object Protocol:** Implement complete AST abstraction representing classes and object hierarchies across guest runtimes.
* :calendar: **Reactive Event Protocol:** Event-driven architecture supporting cross-runtime reactive callbacks.
* :calendar: **Full C++ Core Migration:** Evaluate migrating internal core implementation to modern C++ while exporting clean C APIs.
* :pause_button: **Driver-Level Sandboxing:** Advanced memory isolation and signature verification driver for loaded scripts.

### CLI & Examples
* :calendar: **Unified CLI:** Common front-end CLI supporting both MetaCall and MetaCall++ runtime interactions.
* :calendar: **Polyglot Production Examples:** Implement advanced polyglot templates (e.g., reactive web front-ends with native C/C++ back-ends, matrix/math solvers bridging Python/NodeJS and C).

### Dropped / Retired
* :x: **Vagrant Workstations:** Dropped in favor of standardized, lightweight Docker and containerized CI environments.
* :x: **Latex & Quickbook Documentation:** Dropped legacy format targets in favor of unified Markdown and modern static site generation.

---

## Tracking & References

* **Superseded Backlog:** Historical items triaged from [TODO.md](TODO.md).
* **Active Milestones:** Track ongoing issues and project tasks on [MetaCall Core Issues](https://github.com/metacall/core/issues) and [EPIC #863](https://github.com/metacall/core/issues/863).