# Python + Valgrind Helpers

This folder contains helper scripts for building Python with Valgrind support in a reproducible environment.

## Scope

- Build debug Python with `--with-valgrind`.
- Run Python-port tests under Valgrind with configurable depth.
- Generate logs suitable for leak triage and sharing.

## Notes

- Prefer containerized runs to avoid host contamination.
- Keep aggressive suppressions opt-in from `tools/instrumentation/suppressions`.
