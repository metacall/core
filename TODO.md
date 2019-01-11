TODO List
=========

## CORE

  * Merge serial, detour and loader systems into a generic plugin system.
  * Improve versioning of loader plugins and it's [interface](https://accu.org/index.php/journals/1718).
  * Extend loading process to accept scripts as string, uri or memory block.
  * Add a cross-platform library for managing path tree and allow scripts to be loaded by path.
  * Add watchers and reload policies in order to provide on-the-fly changes and make it fault tolerant against corrupted scripts.
  * Add error management.
  * Add new type management and its implementation in each loader (ptr, array, callback, object).
  * Improve scope management for encapsulation and add tie-breaker to avoid conflicts between functions with same name.
  * **MetaCall++** front-end compatible with SWIG (avoid common exceptions and unimplemented templates).
  * Possibly convert the whole project to C++ and export critical parts like **MetaCall** front-end and loaders plugin interface as [C API](http://www.drbobbs.com/cpp/building-your-own-plugin-framework-part/204202899?pgno=4).
  * Implement a meta-object protocol in order to support a complete AST abstraction representing class and object tree.
  * Implement an event protocol for supporting reactive architectures based on callbacks.
  * Make an automated architecture and test generation for implement all tests with a workflow (TDD) and similar semantics, and implement them.
  * Add custom benchmarks and small utility library for supporting benchmarks, and integrate it in build process of CMake.
  * Improve security of loaders and scripts loaded with signatures, and avoid memory injection with a driver.
  * Implement thread safety.

## LOADERS

  * Loader C/C++:
    - [Clang](http://clang.llvm.org/docs/ExternalClangExamples.html) as a parser for reflect generation for .h and .hpp files.
    - [Libffi](http://www.chiark.greenend.org.uk/doc/libffi-dev/html/Using-libffi.html) as a foreing function interface caller.
    - [LLVM](http://llvm.org/docs/) as a JIT compiler for .c and .cpp files.
    - [Dynlink](source/dynlink) library as a cross-platform library loader.
    - Alternatively use [EmbedCh](https://www.softintegration.com/products/sdk/embedch/).

  * Loader C#:
    - [NET Core](https://msdn.microsoft.com/en-us/library/ms404385.aspx) hosting.
    - [Mono](http://www.mono-project.com/docs/advanced/embedding/).

  * Loader Java:
    - [JVM and JNI](http://docs.oracle.com/javase/1.5.0/docs/guide/jni/spec/invocation.html).

  * Loader PHP:
    - [PHP C API](http://us.php.net/manual/en/internals2.php).

  * Loader LUA:
    - [LUA C API](https://www.lua.org/pil/contents.html#P4).

## DOCUMENTATION

  * Write a complete documentation of the core source and libraries.
  * Write a separated text with a complete manual of each example.
  * Integrate automated generation of documentation with CMake build:
    - [Doxygen](https://www.stack.nl/~dimitri/doxygen/manual/docblocks.html) for extracting code documentation.
    - [Markdown](https://www.stack.nl/~dimitri/doxygen/manual/markdown.html) for general purpose repoistory based documentation.
    - [Latex](https://cmake.org/Wiki/CMakeUserUseLATEX) for custom examples and manuals. -- [Already integrated](docs/manual/cmake-init.tex).
    - [Quickbook](http://www.boost.org/doc/libs/1_61_0/html/quickbook.html) for integrating a complete set of automating tools.

## EXAMPLES

  * Simple example of a front-end web-page (Js with Angular or React) with a logic based on (C/C++).
  * Extension of first example providing a custom service as mark-water implemented natively and a custom database layer implementing a persistent cache.
  * Custom threading, high performance example based on script host language (Python, Ruby, Js) and a guest native language (C/C++), to implement a matrix based solver (Sudoku or some Math problem).
  * Implement an example of software migration from an old software language to a newer one (possibly Visual Basic 6 to some script).
  * Implement a CLI for **MetaCall++** and extend current CLI for **MetaCall** (possibly with a common front-end and different adaptors for the back-end).

## DEPLOY

  * Integrate project with SWIG and provide automated port generation for **MetaCall** and **MetaCall++**.
  * Add continuous integration for multiple architectures and operative systems:
    - Docker for deploying.
    - Vagrant for generating workstations for developers.
    - Set up scripts (sh, bat) for multiple architectures for each CI container.
    - CI/CD release of .deb and .rpm packages
  * Add cross-compiling toolkit for multiple architectures integrated with [CMake](https://cmake.org/cmake/help/v3.0/manual/cmake-toolchains.7.html)

## MISC

  * Standarize code style guidelines and tabulation (editorconfig and editorconfig tools).
  * Implement a linter step in build process for style guidelines, static analysis, etc... for C/C++ code (possibly [Boost](https://svn.boost.org/trac/boost/wiki/BestPracticeHandbook) style).
