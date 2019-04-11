
<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%;" width="32" height="32">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**METACALL** is a library that allows calling functions, methods or procedures between programming languages. With **METACALL** you can transparently execute code from / to any programming language, for example, call Python code from JavaScript code.

`sum.py`
``` python
def sum(a, b):
  return a + b
```

`main.js`
``` javascript
metacall_load_from_file('py', [ 'sum.py' ]);

metacall('sum', 3, 4); // 7
```

# Table Of Contents

<!-- TOC -->

- [Abstract](#abstract)
- [Table Of Contents](#table-of-contents)
  - [1. Motivation](#1-motivation)
  - [2. Language Support (Backends)](#2-language-support-backends)
  - [3. Use Cases](#3-use-cases)
  - [4. Usage](#4-usage)
  - [4.1 Installation](#41-installation)
    - [4.2 Environment Variables](#42-environment-variables)
    - [4.3 Examples](#43-examples)
  - [5. Architecture](#5-architecture)
    - [5.1 Overview](#51-overview)
      - [5.1.1 Design Decisions](#511-design-decisions)
      - [5.1.2 Modules](#512-modules)
    - [5.2 Reflect](#52-reflect)
      - [5.2.1 Type System](#521-type-system)
      - [5.2.2 Values](#522-values)
      - [5.2.3 Functions](#523-functions)
    - [5.3 Plugins](#53-plugins)
      - [5.3.1 Loaders](#531-loaders)
        - [5.3.1.1 Python](#5311-python)
        - [5.3.1.2 NodeJS](#5312-nodejs)
        - [5.3.1.3 JavaScript](#5313-javascript)
        - [5.3.1.4 C#](#5314-c)
        - [5.3.1.5 Ruby](#5315-ruby)
        - [5.3.1.6 Mock](#5316-mock)
        - [5.3.1.7 File](#5317-file)
      - [5.3.2 Serials](#532-serials)
        - [5.3.2.1 MetaCall](#5321-metacall)
        - [5.3.2.2 RapidJSON](#5322-rapidjson)
      - [5.3.3 Detours](#533-detours)
        - [5.3.3.1 FuncHook](#5331-funchook)
    - [5.4 Ports](#54-ports)
    - [5.5 Serialization](#55-serialization)
    - [5.6 Memory Layout](#56-memory-layout)
    - [5.7 Fork Model](#57-fork-model)
    - [5.8 Threading Model](#58-threading-model)
  - [5. Application Programming Interface (API)](#5-application-programming-interface-api)
  - [6. Build System](#6-build-system)
    - [6.1 Build Options](#61-build-options)
  - [7. Platform Support](#7-platform-support)
    - [7.1 Docker Support](#71-docker-support)
  - [8. License](#8-license)

<!-- /TOC -->

## 1. Motivation

The **METACALL** project started time ago when I was coding a [Game Engine for an MMORPG](https://bitbucket.org/parrastudios/argentum-online-c). My idea was to provide an interface to allow other programmers extend the Game Engine easily. By that time, I was finishing the university so I decide to do my [Final Thesis](https://bitbucket.org/parrastudios/argentum-online-c/raw/e6e78fef80c6adc541640d68d422721ef735184f/common/doc/Plugin/plugin-framework-paper.pdf) and [Presentation](https://bitbucket.org/parrastudios/argentum-online-c/raw/e6e78fef80c6adc541640d68d422721ef735184f/common/doc/Plugin/plugin-framework-presentation.pdf) based on the plug-in system for my Game Engine. The Plugin Architecture designed for the Game Engine has similarities with **METACALL** although the architecture has been redefined and the code has been rewritten from scratch. After some refination of the system, I came up with **METACALL** and other use cases for the tool. Currently we are using **METACALL** to build a cutting edge FaaS (Function as a Service) **[https://metacall.io](https://metacall.io/)** based on this technique to provide high scalability of the functions among multiple cores and **[Function Mesh](https://medium.com/@metacall/function-mesh-architecture-c0304ba4bad0)** pattern, a new technique I have developed to interconnect transparently functions in a distributed system based on this library.

## 2. Language Support (Backends)

This section describes all programming languages that **METACALL** supports, if you are interested in from what languages can be used **METACALL** you must go to [ports section](#54-ports).

- Currently supported languages and run-times:

| Language                                                           | Runtime                                                                                      |         Version         | Tag  |
|--------------------------------------------------------------------|----------------------------------------------------------------------------------------------|:-----------------------:|:----:|
| [Python](https://www.python.org/)                                  | [Python C API](https://docs.python.org/3/c-api/intro.html)                                   |    **>= 3.2 <= 3.6**    |  py  |
| [NodeJS](https://nodejs.org/)                                      | [N API](https://nodejs.org/api/n-api.html)                                                   | **>= 8.11.1 <= 8.12.0** | node |
| [JavaScript](https://developer.mozilla.org/bm/docs/Web/JavaScript) | [V8](https://v8.dev/)                                                                        |       **5.1.117**       |  js  |
| [C#](https://dotnet.microsoft.com/)                                | [NetCore](https://github.com/dotnet/docs/blob/master/docs/core/tutorials/netcore-hosting.md) |       **1.1.10**        |  cs  |
| [Ruby](https://ruby-lang.org/)                                     | [Ruby C API](https://silverhammermba.github.io/emberb/c/)                                    |    **>= 2.1 <= 2.3**    |  rb  |
| [Mock](/source/loaders/mock_loader)                                | **∅**                                                                                        |        **0.1.0**        | mock |

- Languages and run-times under construction:

| Language                                                           | Runtime                                                                                                | Tag  |
|--------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------|:----:|
| [Java](https://www.java.com/)                                      | [JNI](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/)                                     | java |
| [C/C++](http://www.cplusplus.com/)                                 | [Clang](https://clang.llvm.org/) - [LLVM](https://llvm.org/) - [libffi](http://sourceware.org/libffi/) |  c   |
| [File](/source/loaders/file_loader)                                | **∅**                                                                                                  | file |
| [Go](https://golang.org/)                                          | Go Runtime                                                                                             |  go  |
| [Haskell](https://www.haskell.org/)                                | [Haskell FFI](https://wiki.haskell.org/GHC/Using_the_FFI)                                              |  hs  |
| [JavaScript](https://developer.mozilla.org/bm/docs/Web/JavaScript) | [SpiderMonkey](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference) | jsm  |

## 3. Use Cases

**METACALL** can be used in the following cases:

- Interconnect different technologies in the same project. It allows to have heterogeneous teams of developers working over same project in an isolated way and using different programming languages at the same time.

- Embedding programming languages to existing softwares. Game Engines, 3D Editors like [Blender](https://www.blender.org/), among others can take benefit of **METACALL** and extend the core functionality with higher level programming languages (aka scripting).

- Function as a Service. **METACALL** can be used to implement efficient FaaS architectures. We are using it to implement our own FaaS (Function as a Service) **[https://metacall.io](https://metacall.io/)** based on **[Function Mesh](https://medium.com/@metacall/function-mesh-architecture-c0304ba4bad0)** pattern and high performance function scalability thanks to this library.

- Source code migrations. **METACALL** can wrap large and legacy code-bases, and provide an agnostic way to work with the codebase into a new programming language. Eventually the code can be migrated by parts, without need of creating a new project or stopping the production environment. Incremental changes can be done, solving the migration easily and with less time and effort.

- Porting low level libraries to high level languages transparently. With **METACALL** you can get rid of extension APIs like Python C API or NodeJS N-API. You can call directly low level libraries from your high level languages without making a wrapper in C or C++ for it.

As you can see, there are plenty of uses. **METACALL** introduces a new model of programming which allows a high interoperability between technologies. If you find any other use case just let us know about it with a Pull Request and we will add it to the list.

## 4. Usage

## 4.1 Installation

Prior to try any example, you must have **METACALL** installed in your system. To install **METACALL** you have the following options.

- [Download a release](/TODO).
- [Install via package manager](/TODO).
- [Build and install it manually](#6-build-system).
- [Pull it from DockerHub](/TODO).

### 4.2 Environment Variables

This environment variables are optional, in case that you want to modify default paths of **METACALL**.

|           Name            | Description                                                      |          Default Value           |
|:-------------------------:|------------------------------------------------------------------|:--------------------------------:|
| **`DETOUR_LIBRARY_PATH`** | Directory where detour plugins to be loaded are located          |          **`detours`**           |
| **`SERIAL_LIBRARY_PATH`** | Directory where serial plugins to be loaded are located          |          **`serials`**           |
| **`CONFIGURATION_PATH`**  | File path where the **METACALL** global configuration is located | **`configurations/global.json`** |
| **`LOADER_LIBRARY_PATH`** | Directory where loader plugins to be loaded are located          |          **`loaders`**           |
| **`LOADER_SCRIPT_PATH`**  | Directory where scripts to be loaded are located                 | **`${execution_path}`** &#x00B9; |

&#x00B9; **`${execution_path}`** defines the path where the program is executed, **`.`** in Linux.

### 4.3 Examples

- [MetaCall CLI](/source/examples/metacallcli). Example of a Command Language Interpreter based on MetaCall where you can load, unload scripts and call their functions.

- [MetaCall Rotulin](https://github.com/metacall/rotulin). Example of a multi-language application built with **METACALL**. This application embeds a Django server with a Ruby DataBase and C# business layer based on ImageMagick.

## 5. Architecture

### 5.1 Overview

#### 5.1.1 Design Decisions

- To provide an high level API with a simple UX and to be easy to understand.

- To work in high performance environments.

- To be as cross-platform as possible.

- To avoid to modify run-times directly or use the code inside **METACALL** in order to avoid maintaining them, or propagating security flaws or licenses into **METACALL**.

- To provide support for any embeddable programming language and to provide support for **METACALL** to be used form any programming language.

- All external code used into **METACALL** must be introduced by inversion of control in the plugin system, so that the core must not remain aware from what software is using.

- All code developed in **METACALL** must be implemented in standalone libraries that can work by itself in an isolated way (aka modules).

#### 5.1.2 Modules

- [`adt`](/source/adt) provides a base for Abstract Data Types and algorithms used in **METACALL**. Implementation must be done in an efficient and generic way. Some of the data structures implemented are vector, set, hash, comparable or trie.

- [`detour`](/source/detour) provides an interface to hook into functions. Detours are used by the [fork model](#57-fork-model) to intercept fork calls.

- [`detours`](/source/detours) implement the [`detour`](/source/detour) interface by using a plugin architecture. The current list of available detour plugins is the following one.
  - [`funchook_detour`](/source/detours/funchook_detour) implemented by means of FuncHook library.

- [`distributable`](/source/distributable) defines the compilation of **METACALL** that generates an unique library with all core libraries bundled into it. As the **METACALL** architecture is divided by modules, in order to distribute **METACALL** is needed to build all of them into a single library. This module implements this compilation by means of CMake.

- [`dynlink`](/source/dynlink) implements a cross-platform method to dynamically load libraries. It is used to dynamically load plugins into **METACALL**.

- [`environment`](/source/environment) implements an standard way to deal with environment variables. **METACALL** uses environment variables to define custom paths for plugins and scripts.

- [`examples`](/source/examples) ...

- [`filesystem`](/source/filesystem) provides an abstraction for operative system file system.

- [`format`](/source/format) provides an standard way for printing to standard input output for old C versions that does not support newest constructions.

- [`loader`](/source/loader) ...

- [`loaders`](/source/loaders)

- [`log`](/source/log)

- [`memory`](/source/memory)

- [`metacall`](/source/metacall)

- [`ports`](/source/ports)

- [`preprocessor`](/source/preprocessor)

- [`reflect`](/source/reflect)

- [`scripts`](/source/scripts)

- [`serial`](/source/serial)

- [`serials`](/source/serials)

- [`tests`](/source/tests)

- [`version`](/source/version)

### 5.2 Reflect

The module that holds the representation of types, values and functions is called [`reflect`](/source/reflect) and it handles the abstraction of code loaded into **METACALL**.

**METACALL** uses reflection and introspection techniques to inspect the code loaded by the [`loaders`](/source/loaders) in order to interpret it and provide an higher abstraction of it. With this higher abstraction **METACALL** can easily inter-operate between languages transparently.

#### 5.2.1 Type System

**METACALL** implements an abstract type system which is a binary representation of the types supported by it. This means that **METACALL** can convert any type of a language to its own type system and back. Each loader is responsible of doing this conversions.

**METACALL** maintains most of the types of the languages but not all are supported. If new types are added they have to be implemented in the [`reflect`](/source/reflect) module and also in the [`loaders`](/source/loaders) and [`serials`](/source/serials) to fully support it.

|  Type   | Value                                                              |
|:-------:|--------------------------------------------------------------------|
| Boolean | `true` or `false`                                                  |
|  Char   | `-128` to `127`                                                    |
|  Short  | `-32,768` to `32,767`                                              |
|   Int   | `-2,147,483,648` to `2,147,483,647`                                |
|  Long   | `–9,223,372,036,854,775,808` to `9,223,372,036,854,775,807`        |
|  Float  | `1.2E-38` to `3.4E+38`                                             |
| Double  | `2.3E-308` to `1.7E+308`                                           |
| String  | NULL terminated list of characters                                 |
| Buffer  | Blob of memory representing a binary data                          |
|  Array  | Arrangement of values of any type                                  |
|   Map   | List of elements formed by a key (String) value (Any) pair (Array) |
| Pointer | Low level representation of a memory reference                     |
|  Null   | Representation of NULL value type                                  |

- Boolean is mostly represented by an integer value. There are languages that does not support it so it gets converted to a integer value in the memory layout.

- Integer and Floating Point values provide a complete abstraction to numerical types. Type sizes are preserved and the correct type is used when using any number. This depends on the internal implementation of the value by the run-time. Although there can be problems related to this. A `bignum` type from Ruby may overflow if it is too big when trying to convert it to a `float` type in C#.

- String is represented by ASCII encoding currently. Future versions will implement multiple encodings to be interoperable between other language encodings.

- Buffer represents a blob of raw memory (i.e. an array of bytes). This can be used to represent files as images or any other resources into memory.

- Array is implemented by means of array of values, which you can think it should be called _list_ instead. But as the memory layout is stored into a contiguous memory block of references to values, it is considered an array.

- Map implements an associative key value pair container. A map is implemented with an array of two sized elements array. Each element of the map is an array of size two, where the first element of it is always an String and the second element is a value of any type.

- Pointer is an opaque value representing a raw reference to a memory block. Some languages allow to use references to memory and some others not. This type is opaque because **METACALL** does not know what kind of concrete value represents it. The representation may be a complex type handled by the developer source code inside the run-time.

- Null type implements a null value. This type has only been implemented in order to support null value from multiple run-times. It represents a null value and it does not have data size on the value allocated.

#### 5.2.2 Values

Values represent the instances of the **METACALL** type system.

The memory layout guarantees to fit at least the same size of the types into memory. This means if a boolean type can be represented with one bit inside a value of one byte size, maybe this value is stored in a bigger memory block and this fact is architecture and platform dependant.

When converting values between different types, if any potential number overflow or invalid conversion between types is done, **METACALL** will warn about it. If any conversion of types can be handled by **METACALL**, it will automatically cast or transform the values into the target type automatically in order to avoid errors in the call.

The value model is implemented by means of object pool. Each value is a reference to a memory block allocated from a memory pool (which can be injected into **METACALL**). The references can be passed by value, this means **METACALL** copies the reference value instead of the data which this reference is pointing to, like most run-times do when managing their own values.

Each created value must be destroyed manually. Otherwise it will lead to a memory leak. This fact only occurs when dealing with **METACALL** at C level. If **METACALL** is being used in an higher language through [`ports`](/source/ports), the developer does not have to care about memory management.

The value memory layout is described in the following form.

| Memory Offset | `0` to `sizeof(data) - 1` | `sizeof(data)` to `sizeof(data) + sizeof(type_id) - 1` |
|:-------------:|:-------------------------:|:------------------------------------------------------:|
|  **Content**  |         **DATA**          |                      **TYPE ID**                       |

This layout is used by the following reasons.

- Data is located at the first position of the memory block, so it can be used as a normal low level value. This allows to threat **METACALL** values as a normal C values. Therefore you can use **METACALL** with normal pointers to existing variables, literal values as shown in the previous examples or **METACALL** values.

- Data can be accessed faster as it is located at first position of the memory block. There is not extra calculation of an offset when trying to access the pointer.

- Data and type id are contiguously allocated in order to threat it as the same memory block so it can be freed with one operation.

#### 5.2.3 Functions

Functions are an abstract callable representation of functions, methods or procedures loaded by [`loaders`](/source/loaders). The functions are like a template who is linked to a loader run-time and allows to do a foreign function call.

A function is composed by a name and a signature. The signature defines the arguments name, type, and return type if any. When a function is loaded, **METACALL** tries to inspect the signature and records the types if any. It stores the arguments name and size and also a concrete type that will be used later by the loader to implement the call to the run-time.

The function interface must be implemented by the [`loaders`](/source/loaders) and it has the following form.

``` c
typedef struct function_interface_type
{
  function_impl_interface_create create;
  function_impl_interface_invoke invoke;
  function_impl_interface_destroy destroy;

} * function_interface;
```

- `create` instantiates the function concrete data related to the run-time.
- `invoke` transforms arguments from [`reflect`](/source/reflect) abstract types to run-time concrete types, executes the call in the run-time, and converts the result of the call from run-time concrete type to [`reflect`](/source/reflect) abstract type.
- `destroy` clears all data previously instantiated in `create`.

The type deduction can be done at different levels. For example, it is possible to guess function types from the loaded code.

``` python
def multiply_type(a: int, b: int) -> int:
  return a * b
```

If this code is loaded, **METACALL** will be able to inspect the types and define the signature. Signature includes the names of the arguments, the types of those arguments if any, and the return type if any.

It may be possible that the function loaded into **METACALL** is duck typed. This means it does not have information about what types it supports and therefore they cannot be inspected statically.

``` python
def multiply_duck(a, b):
  return a * b
```

At low level **METACALL** must always know the types to do the call. This types can be inferred statically or dynamically and this has implications over the call model.

In the first example, we can simply call the function without specifying the types.

``` c
metacall("multiply_type", 3, 4); // 12
```

As the signature is already know the literal values `3` and `4` can be converted into **METACALL** values automatically. Note that in this case, as literal values are provided, if we pass a double floating point, the memory representation of the value will be corrupted as there is no possible way to detect input values and cast them to the correct target values.

In the second example, the values are not know. If we use the same API to call the function, **METACALL** will not be able to call correctly the function as its types are not know. To allow calls to duck typed functions the developer must specify the value types he is passing to the function.

``` c
const enum metacall_value_id multiply_types[] =
{
  METACALL_INT, METACALL_INT
};

metacallt("multiply_duck", multiply_types, 3, 4); // 12
```

This method allows to pass different value types to the same function. The following call would be valid too.

``` c
const enum metacall_value_id multiply_types[] =
{
  METACALL_DOUBLE, METACALL_DOUBLE
};

metacallt("multiply_duck", multiply_types, 3.0, 4.0); // 12.0
```

### 5.3 Plugins

**METACALL** has a plugin architecture implemented at multiple levels.

- Loaders implement a layer of plugins related to the run-times.

- Serials implement a layer of (de)serializers in order to transform input (arguments) or output (return value) of the calls into a generic format.

- Detours is another layer of plugins focused on low level function interception (hooks).

Each plugin is a piece of software that can be dynamically loaded into the **METACALL** core, used and unloaded when it is not needed anymore.

#### 5.3.1 Loaders

Loaders are responsible for embedding run-times into **METACALL**. Each loader has the following interface.

``` c
typedef struct loader_impl_interface_type
{
  loader_impl_interface_initialize initialize;
  loader_impl_interface_execution_path execution_path;
  loader_impl_interface_load_from_file load_from_file;
  loader_impl_interface_load_from_memory load_from_memory;
  loader_impl_interface_load_from_package load_from_package;
  loader_impl_interface_clear clear;
  loader_impl_interface_discover discover;
  loader_impl_interface_destroy destroy;

} * loader_impl_interface;
```

A loader must implement it to be considered a valid loader.

- `initialize` starts up the run-time.
- `execution_path` defines a new import path to the run-time.
- `load_from_file` loads a code from file into the run-time and returns a handle which represents it.
- `load_from_memory` loads a code from memory into the run-time and returns a handle which represents it.
- `load_from_package` loads a code from a compiled library or package into the run-time and returns a handle which represents it.
- `clear` unloads a handle from the run-time.
- `discover` inspects a handle previously loaded.
- `destroy` shutdowns the run-time.

##### 5.3.1.1 Python

##### 5.3.1.2 NodeJS

##### 5.3.1.3 JavaScript

##### 5.3.1.4 C# #

##### 5.3.1.5 Ruby

##### 5.3.1.6 Mock

##### 5.3.1.7 File

#### 5.3.2 Serials

##### 5.3.2.1 MetaCall

##### 5.3.2.2 RapidJSON

#### 5.3.3 Detours

##### 5.3.3.1 FuncHook

### 5.4 Ports

### 5.5 Serialization

### 5.6 Memory Layout

### 5.7 Fork Model

**METACALL** implements a fork safe model. This means if **METACALL** is running in any program instance, thr process where is running can be forked safely at any moment of the execution. This fact has many implications at design, implementation and use level. But the whole **METACALL** architecture tries to remove all responsibility from the developer and make this transparent.

To understand the **METACALL** fork model, first of all we have to understand the implications of the forking model in operative systems and the difference between [fork-one and fork-all models](https://docs.oracle.com/cd/E37838_01/html/E61057/gen-1.html).

The main difference between fork-one and fork-all is that in fork-one only the thread which called the fork is preserved after the fork (i.e. gets cloned). In fork-all model, all threads are preserved after cloning. POSIX uses fork-one model, meanwhile Oracle Solaris use the fork-all model.

Because of fork-one model, forking a running run-time like NodeJS (which has a thread pool) implies that in the child process the thread pool will be almost dead except the thread which did the fork call. So NodeJS run-time cannot continue the execution anymore and the event-loop enters into a deadlock state.

When a fork is done, the status of the execution is lost by the moment. **METACALL** is not able to preserve the state when a fork is done. Some run-times do not allow to preserve the internal state. For example, the bad design of NodeJS does not allow to manage the thread pool from outside, so it cannot be preserved after a fork.

Because of these restrictions, **METACALL** cannot preserve the status of the run-times. In the future this model will be improved to maintain consistency and preserve the execution state of the run-times making **METACALL** more robust.

Although the state is not preserved, fork safety is. The mechanism **METACALL** uses to allow fork safety is described in the following enumeration.

1) Intercept fork call done by the program where **METACALL** is running.

2) Shutdown all run-times by means of unloading all loaders.

3) Execute the real fork function.

4) Restore all run-times by means of reloading all loaders.

5) Execute user defined fork callback if any.

To achieve this, **METACALL** hooks fork primitives depending on the platform.

- `fork` on POSIX systems.
- `RtlCloneUserProcess` on Windows systems.

If you use `clone` instead of `fork` to spawn a new process in a POSIX system, **METACALL** won't catch it.

Whenever you call a to a cloning primitive **METACALL** intercepts it by means of [**`detour`**](/source/detour). Detours is a way to intercept functions at low level by editing the memory and introducing a jump over your own function preserving the address of the old one. **METACALL** uses this method instead of POSIX `pthread_atfork` for three main reasons.

- The first one is that `pthread_atfork` is only supported by POSIX systems. So it is not a good solution because of the philosophy of **METACALL** is to be as cross-platform as possible.

- The second is that `pthread_atfork` has a [bug in the design of the standard](https://stackoverflow.com/a/6605487). It was designed to solve a problem which cannot be solved with `pthread_atfork` itself. This means that even having the control of NodeJS thread pool, it will not be possible to restore the [mutexes](https://github.com/nodejs/node/blob/v8.x/src/node_platform.cc) in the child process. The only possibility is to re-implement the thread pool of NodeJS with async safe primitives like a semaphore. Async safe primitives will be able to work in the child process handler. But this is not possible as it enters in conflict with the design decision of to not modify the run-times.

- The third one is that the mechanism of `pthread_atfork` also [will be deprecated](http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_atfork.html) because of second reason.
  > The `pthread_atfork()` function may be formally deprecated (for example, by shading it OB) in a future version of this standard.

Detours model is not safe. It is platform dependant and implies that the program modifies the memory of itself during the execution which is not safe at all and can induce bugs or security flaws if it is not done correctly. But because of limitations of run-times, there is not another alternative to solve the problem of fork safety.

Usually the developer is the same who does the fork, but it may be possible that **METACALL** is embedded into a larger application and the developer is in the middle between the application code and **METACALL** so it is impossible to control when a fork is done. Because of this the developer can register a callback by means of [**`metacall_fork`**](/source/metacall/include/metacall/metacall_fork.h) to know when a fork is executed to do the actions needed after the fork, for example, re-loading all previous code and restore the state of the run-times. This gives a partial solution to the problem of losing the state when doing a fork.

### 5.8 Threading Model

## 5. Application Programming Interface (API)

## 6. Build System

Follow these steps to build and install **METACALL** manually.

``` sh
git clone --recursive https://github.com/metacall/core.git
mkdir core/build && cd core/build
cmake ..
make
make test
sudo make install
```

### 6.1 Build Options

These options can be set using **`-D`** prefix when configuring CMake. For example, the following configuration enables the build of Python and Ruby loaders.

``` sh
cmake -DOPTION_BUILD_LOADERS_PY=On -DOPTION_BUILD_LOADERS_RB=On ..
```

Available build options are the following ones.

|        Build Option         | Description                                            | Default Value |
|:---------------------------:|--------------------------------------------------------|:-------------:|
|    **BUILD_SHARED_LIBS**    | Build shared instead of static libraries.              |      ON       |
| **OPTION_BUILD_DIST_LIBS**  | Build all libraries into a single compilation unit.    |      ON       |
|  **OPTION_SELF_CONTAINED**  | Create a self-contained install with all dependencies. |      OFF      |
|   **OPTION_BUILD_TESTS**    | Build tests.                                           |      ON       |
| **OPTION_BUILD_BENCHMARKS** | Build benchmarks.                                      |      OFF      |
|    **OPTION_BUILD_DOCS**    | Build documentation.                                   |      OFF      |
|  **OPTION_BUILD_EXAMPLES**  | Build examples.                                        |      ON       |
|  **OPTION_BUILD_LOADERS**   | Build loaders.                                         |      ON       |
|  **OPTION_BUILD_SCRIPTS**   | Build scripts.                                         |      ON       |
|  **OPTION_BUILD_SERIALS**   | Build serials.                                         |      ON       |
|  **OPTION_BUILD_DETOURS**   | Build detours.                                         |      ON       |
|   **OPTION_BUILD_PORTS**    | Build ports.                                           |      OFF      |
|    **OPTION_FORK_SAFE**     | Enable fork safety.                                    |      OFF      |
|   **OPTION_THREAD_SAFE**    | Enable thread safety.                                  |      OFF      |
|    **CMAKE_BUILD_TYPE**     | Define the type of build.                              |    Release    |

It is possible to enable or disable concrete loaders, script, ports, serials or detours. For building use the following options.

|    Build Option Prefix    | Build Option Suffix                                                   |
|:-------------------------:|-----------------------------------------------------------------------|
| **OPTION_BUILD_LOADERS_** | `C` `JS` `CS` `MOCK` `PY` `JSM` `NODE` `RB` `JSM` `FILE`              |
| **OPTION_BUILD_SCRIPTS_** | `C` `CS` `JS` `NODE` `PY` `RB` `JAVA`                                 |
| **OPTION_BUILD_SERIALS_** | `METACALL` `RAPID_JSON`                                               |
| **OPTION_BUILD_DETOURS_** | `FUNCHOOK`                                                            |
|  **OPTION_BUILD_PORTS_**  | `CS` `CXX` `D` `GO` `JAVA` `JS` `LUA` `NODE` `PHP` `PL` `PY` `R` `RB` |

## 7. Platform Support

The following platforms and architectures have been tested an work correctly with all plugins of **METACALL**.

|     Operative System      |    Architecture     |    Compiler     |                                              Build Status                                               |
|:-------------------------:|:-------------------:|:---------------:|:-------------------------------------------------------------------------------------------------------:|
|    **`ubuntu:xenial`**    |     **`amd64`**     |    **`gcc`**    |                                                                                                         |
| **`debian:stretch-slim`** |     **`amd64`**     | **`gcc:6.3.0`** | [![build](https://gitlab.com/metacall/core/badges/master/build.svg)](https://gitlab.com/metacall/core) |
| **`debian:buster-slim`**  |     **`amd64`**     | **`gcc:8.2.0`** |                                                                                                         |
|       **`windows`**       | **`x86`** **`x64`** |   **`msvc`**    |                                                                                                         |

### 7.1 Docker Support

To provide a reproducible environment **METACALL** is also distributed under Docker on [DockerHub](https://hub.docker.com/r/metacall/core). Current images are based on `debian:stretch-slim` for `amd64` architecture.

For pulling the **METACALL** `latest` image containing the runtime, use:

``` sh
docker pull metacall/core
```

For pulling a specific image depending on the tag, use:

- **METACALL** `deps` image. Includes all dependencies for development:

``` sh
docker pull metacall/core:deps
```

- **METACALL** `dev` image. Includes all dependencies, headers and libraries for development:

``` sh
docker pull metacall/core:dev
```

- **METACALL** `runtime` image. Includes all dependencies and libraries for runtime (equivalent to `latest`):

``` sh
docker pull metacall/core:runtime
```

## 8. License

**METACALL** is licensed under **[Apache License Version 2.0](/LICENSE)**.

>Copyright (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <<vic798@gmail.com>>
>
>Licensed under the Apache License, Version 2.0 (the "License");
>you may not use this file except in compliance with the License.
>You may obtain a copy of the License at
>
>       http://www.apache.org/licenses/LICENSE-2.0
>
>Unless required by applicable law or agreed to in writing, software
>distributed under the License is distributed on an "AS IS" BASIS,
>WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
>See the License for the specific language governing permissions and
>limitations under the License.
