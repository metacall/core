
# Security Policy

## Where To Report a Vulnerability

If you think you have found a vulnerability in this repository, please report it to us through coordinated disclosure.

**Please do not report security vulnerabilities through public issues, discussions, or change requests.**

Instead, report it using one of the following ways:

* Report a [vulnerability](https://github.com/metacall/core/security/advisories/new) directly via private vulnerability reporting on GitHub.
* As a fallback, [send an email](contact@metacall.io) to us.

## What We Consider a Security Vulnerability

MetaCall Core repository is oriented to low level and mainly coded in C/C++ so vulnerabilities are focused mainly on this area:

* Heap/stack/global buffer overflows and out-of-bounds access
* Use-after-free and double-free
* Invalid or uninitialized memory access
* Integer overflow/underflow leading to memory corruption
* Type mismatch (mixing wrong sized integers, floats or passing wrong types between languages)
* Unsafe casts or ABI/FFI violations
* Function-pointer corruption or incorrect dispatch
* Race conditions causing memory corruption or privilege/security-boundary violations
* Arbitrary code execution where code execution was not already authorized by the caller
* Arbitrary file read/write caused by MetaCall
* Path traversal or unsafe dynamic-library/plugin loading
* Malicious or malformed configuration causing security impact
* Serialization/deserialization/type-conversion vulnerabilities
* Sandbox/isolation escapes, where an actual isolation boundary exists
* Denial of service caused by attacker-controlled input
* Information disclosure caused by memory or runtime-boundary violations

Note that MetaCall Core is designed to execute code and load language runtimes. Code execution or library loading that is explicitly requested by the application is not itself a vulnerability. A security issue exists when an attacker can cause MetaCall to execute code, load a library, access data, or perform another privileged operation beyond what the application intended.

Finally, a crash, assertion failure, sanitizer finding, memory leak, or undefined behavior is not automatically a security vulnerability. Reports should explain how an attacker could influence the condition and what security impact it has. If the vulnerability cannot be exploited by an attacker it is considered a normal error or bug and it can be reported in the [issue tracker](https://github.com/metacall/core/issues) as usual.

## How to Report a Vulnerability

1) The following information must be filled in order to consider your vulnerability report valid:

* MetaCall version, release/tag, or commit
* Operating system and architecture
* Compiler and compiler version
* Build configuration/CMake options
* Relevant loader, port, serial or detour
* Minimal reproduction case
* Input or file required to trigger the issue
* Exact steps to reproduce
* Stack trace/backtrace if available
* ASan/UBSan/TSan/MSan/Valgrind output, if available
* Expected vs actual behavior
* Security impact and attacker-controlled input
* Whether the issue is reproducible
* Proof-of-concept or exploit, if available

2) The most important part is to be able to reproduce the issue, if it is possible. If we cannot reproduce the issue it will be hard to provide a solution for it. We follow Test Driven Development so providing the exact code that reproduces the issue will help us to create a test and implement a solution that. For example, someone reporting:

> ASan reports heap-use-after-free in detour_unload() on Linux x86_64, GCC 15, OPTION_FORK_SAFE=ON, under the following code:
> 
> ```c
> // Minimal reproducible example triggering the issue:
> detour_unload(d);
> // ...
> ```

is giving the maintainers something immediately actionable.

A plain:

> MetaCall crashes sometimes

is not.

3) When reporting, each vulnerability must be classified as one of the following levels:

| Severity | Examples |
| :--- | :--- |
| **Critical** | Remote/attacker-controlled code execution, sandbox escape, major cross-boundary compromise |
| **High** | Exploitable memory corruption, arbitrary file access, privilege/security-boundary bypass |
| **Medium** | Significant information disclosure, attacker-triggered DoS, exploitable race/logic flaw |
| **Low** | Limited information disclosure, difficult-to-exploit security weaknesses |


## Response SLA

Vulnerabilities will be managed in the following timeline:

* **Acknowledgement**: within 7 business days
* **Initial assessment**: within 15 business days
* **Critical issues**: handled as a priority
* **Disclosure**: coordinated with the reporter and normally after a fix is available
* **Public disclosure**: GitHub Security Advisory, with CVE when appropriate

## Disclosure

For confirmed vulnerabilities affecting released versions, we will follow the next steps:

1. Private investigation.
2. Fix developed and tested.
3. Patch released.
4. GitHub Security Advisory published.
5. CVE requested when appropriate.
6. Changelog/release notes mention the security fix.

## Supported Versions

Only the following versions will be supported:

* Latest stable release
* Previous releases only when practical
* Development branches unsupported

## Security Research / Safe Harbor

We will not pursue legal action against researchers who act in good faith, avoid accessing or modifying data that does not belong to them, avoid service disruption, and report vulnerabilities through this policy.
