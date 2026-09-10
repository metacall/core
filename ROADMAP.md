# MetaCall Roadmap

> High-level technical direction, feature milestones, and architecture roadmap for the MetaCall polyglot runtime.

---

## Horizon Overview

| Status Icon | Meaning |
| :---: | :--- |
| :white_check_mark: | **Done** (Implemented / Shipped) |
| :hammer_and_wrench: | **In Progress** (Active implementation) |
| :calendar: | **Planned** (Scoped for upcoming cycles) |
| :pause_button: | **Deferred** (Postponed / Awaiting prerequisites) |

---

## Now (0 - 3 Months)

### Core & Reflection
* :hammer_and_wrench: **Error Handling Unification:** Consistent cross-language exception propagation and error structures.
* :hammer_and_wrench: **Thread Safety Hardening:** Race condition fixes across runtime borders and plugin interfaces.
* :calendar: **Scope & Lifetimes:** Explicit memory ownership management across runtime boundaries.

### Documentation & Health (Project Rosetta)
* :hammer_and_wrench: **Modern Documentation Infrastructure:** Automated doc build pipeline and tier documentation (Rosetta-1).
* :white_check_mark: **Community Health Files:** Standardized contributing guidelines, issue workflows, and code styles (Rosetta-4).
* :white_check_mark: **Governance & Policies:** Public security policies, supported versions, and updated roadmap (Rosetta-5, Rosetta-6).

### Build & CI
* :hammer_and_wrench: **Sanitizers & Modern Toolchains:** CI workflows enforcing AddressSanitizer and ThreadSanitizer checks.
* :calendar: **C23 / C++23 Modernization:** Adoption of modern standards across native core components.

---

## Next (3 - 12 Months)

### Loaders & Ports
* :calendar: **Loader Backlog Triage:** Improve performance and compatibility for NodeJS, Python, and Ruby loaders.
* :calendar: **Tier Promotion:** Advance experimental loaders through defined stability tiers (Rosetta-3).
* :calendar: **SWIG Ports Overhaul:** Modernize language bindings and eliminate legacy glue code.

### Core Runtime
* :calendar: **Core Plugin Merge:** Unification of core plugin mechanisms into the main runtime.
* :calendar: **Dynamic Watchers:** Hot-reload and file system watchers for active multi-language execution paths.
* :calendar: **Type System Refactoring:** Zero-copy type conversions for composite objects across languages.

### CLI & Usability
* :calendar: **MetaCall CLI Enhancements:** Better packaging, command-line ergonomics, and multi-language interactive REPL.

---

## Later (12+ Months)

### Long-term Architecture
* :calendar: **MetaCall++:** Next-generation modern C++ wrapper layer with idiomatic modern interfaces.
* :pause_button: **Distributed Runtime Bridges:** Networked IPC loaders for distributed polyglot execution.

### Dropped / Retired Features
* :x: **Vagrant Environments:** Dropped in favor of standardized, lightweight Docker workflows.

---

## References & Tracking

* **Legacy Notes:** Historical roadmap items in [TODO.md](TODO.md) have been triaged and superseded by this document.
* **Milestones & Issues:** Track specific tasks under [MetaCall Issues](https://github.com/metacall/core/issues) and [Milestones](https://github.com/metacall/core/milestones).