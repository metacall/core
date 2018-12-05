
<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%;" width="32" height="32">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**METACALL** is a library that allows calling functions, methods or procedures between programming languages. With **METACALL** you can execute transparently code from / to any programming language, for example, call Python code from JavaScript code.

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
  - [2. Language Support](#2-language-support)
  - [3. Use Cases](#3-use-cases)
  - [4. Architecture](#4-architecture)
    - [4.1 Overview](#41-overview)
    - [4.2 Reflect](#42-reflect)
      - [4.2.1 Type System](#421-type-system)
      - [4.2.2 Values](#422-values)
      - [4.2.3 Functions](#423-functions)
    - [4.3 Loaders](#43-loaders)
      - [4.3.1 Python](#431-python)
      - [4.3.2 NodeJS](#432-nodejs)
      - [4.3.3 JavaScript](#433-javascript)
      - [4.3.4 C#](#434-c)
      - [4.3.5 Ruby](#435-ruby)
      - [4.3.6 Mock](#436-mock)
      - [4.3.7 File](#437-file)
    - [4.4 Ports](#44-ports)
    - [4.5 Serialization](#45-serialization)
    - [4.6 Memory Layout](#46-memory-layout)
    - [4.7 Fork Model](#47-fork-model)
    - [4.8 Threading Model](#48-threading-model)
  - [5. Platform Support](#5-platform-support)

<!-- /TOC -->

## 1. Motivation

The **METACALL** project started time ago when I was coding a [Game Engine for an MMORPG](https://bitbucket.org/parrastudios/argentum-online-c). My idea was to provide an interface to allow other programmers extend the Game Engine easily. By that time, I was finishing the university so I decide to do my [Final Thesis]() based on the plug-in system for my Game Engine. After some refination of the system, I came up with **METACALL** and other use cases for the tool. Currently we are using **METACALL** to build a cutting edge [FaaS](https://metacall.io/) (Function as a Service) based on this technique to provide high scalability of the functions among multiple cores or **Function Mesh** pattern, a new technique I have developed to interconnect transparently functions in a distributed system based on this library.

## 2. Language Support

- Currently supported languages and run-times:

| Language                                                           | Runtime                                                                                      | Version           | Tag  |
|--------------------------------------------------------------------|----------------------------------------------------------------------------------------------|-------------------|------|
| [Python](https://www.python.org/)                                  | [Python C API](https://docs.python.org/3/c-api/intro.html)                                   | **>= 3.2 <= 3.6** | py   |
| [NodeJS](https://nodejs.org/)                                      | [N API](https://nodejs.org/api/n-api.html)                                                   | **8.11.1**        | node |
| [JavaScript](https://developer.mozilla.org/bm/docs/Web/JavaScript) | [V8](https://v8.dev/)                                                                        | **5.1.117**       | js   |
| [C#](https://dotnet.microsoft.com/)                                | [NetCore](https://github.com/dotnet/docs/blob/master/docs/core/tutorials/netcore-hosting.md) | **1.1.10**        | cs   |
| [Ruby](https://ruby-lang.org/)                                     | [Ruby C API](https://silverhammermba.github.io/emberb/c/)                                    | **>= 2.1 <= 2.3** | rb   |
| [Mock](/source/loaders/mock_loader)                                | **∅**                                                                                        | **0.1.0**         | mock |

- Languages and run-times under construction:

| Language                                                           | Runtime                                                                                                                                                       | Tag  |
|--------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------|------|
| [Java](https://www.java.com/)                                      | [JNI](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/)                                                                                            | java |
| [C/C++](http://www.cplusplus.com/)                                 | [Clang](https://clang.llvm.org/) - [LLVM](https://llvm.org/) - [libffi](http://sourceware.org/libffi/) / [libffcall](https://www.gnu.org/software/libffcall/) | c    |
| [File](/source/loaders/file_loader)                                | **∅**                                                                                                                                                         | file |
| [Go](https://golang.org/)                                          | Go Runtime                                                                                                                                                    | go   |
| [JavaScript](https://developer.mozilla.org/bm/docs/Web/JavaScript) | [SpiderMonkey](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference)                                                        | jsm  |

## 3. Use Cases

**METACALL** can be used for the following use cases:

- Interconnect different technologies in the same project. It allows to have heterogeneous teams of developers working over same project in an isolated way and using different programming languages at the same time.

- Embedding programming languages to existing softwares. Game Engines, 3D Editors like [Blender](https://www.blender.org/), among others can take benefit of **METACALL** and extend the core functionality with higher level programming languages (aka scripting).

- Function as a Service. **METACALL** can be used to implement efficient FaaS architectures. We are using it to implement our own [FaaS](https://metacall.io/) based on **Function Mesh** pattern and high performance function scalability thanks to this library.

- Source code migrations. **METACALL** can wrap large and legacy code-bases, and provide an agnostic way to work with the codebase into a new programming language. Eventually the code can be migrated by parts, without need of creating a new project or stopping the production environment. Incremental changes can be done, solving the migration easily and with less time and effort.

- Porting low level libraries to high level languages transparently. With **METACALL** you can get rid of extension APIs like Python C API or NodeJS N-API. You can call directly low level libraries from your high level languages without making a wrapper in C or C++ for it.

As you can see, there are plenty of uses. **METACALL** introduces a new model of programming which allows an high interoperability between technologies. If you find any other use case just let us know about it with a Pull Request and we will add it to the list.

## 4. Architecture

### 4.1 Overview

### 4.2 Reflect

The module that holds the representation of types, values and functions is called [`reflect`](/source/reflect) and it handles the abstraction of code loaded into **METACALL**.

**METACALL** uses reflection and introspection techniques to inspect the code loaded by the [`loaders`](/source/loaders) in order to interpret it and provide an higher abstraction of it. With this higher abstraction **METACALL** can easily inter-operate between languages transparently.

#### 4.2.1 Type System

**METACALL** implements an abstract type system which is a binary representation of the types supported by it. This means that **METACALL** can convert any type of a language to its own type system and back. Each loader is responsible of doing this conversions.

**METACALL** maintains most of the types of the languages but not all are supported. If new types are added they have to be implemented in the [`reflect`](/source/reflect) module and also in the [`loaders`](/source/loaders) and [`serials`](/source/serials) to fully support it.

| Type    | Value                                                              |
|---------|--------------------------------------------------------------------|
| Boolean | `true` or `false`                                                  |
| Char    | `-128` to `127`                                                    |
| Short   | `-32,768` to `32,767`                                              |
| Int     | `-2,147,483,648` to `2,147,483,647`                                |
| Long    | `–9,223,372,036,854,775,808` to `9,223,372,036,854,775,807`        |
| Float   | `1.2E-38` to `3.4E+38`                                             |
| Double  | `2.3E-308` to `1.7E+308`                                           |
| String  | NULL terminated list of characters                                 |
| Buffer  | Blob of memory representing a binary data                          |
| Array   | Arrangement of values of any type                                  |
| Map     | List of elements formed by a key (String) value (Any) pair (Array) |
| Pointer | Low level representation of a memory reference                     |

The memory layout guarantees to fit at least the same size of the types into memory. This means if a boolean type can be represented with one bit (inside one byte), maybe this value is stored in a bigger memory block and this fact is architecture and platform dependant.

- Boolean is mostly represented by an integer value. There are languages that does not support it so it gets converted to a integer value in the memory layout.

- Integer and Floating Point values provide a complete abstraction to numerical types. Type sizes are preserved and the correct type is used when using any number. This depends on the internal implementation of the value by the run-time. Although there can be problems related to this. A `bignum` type from Ruby may overflow if it is too big when trying to convert it to a `float` type in C#.

- String is represented by ASCII encoding currently. Future versions will implement multiple encodings to be interoperable between other language encodings.

- Buffer represents a blob of raw memory (i.e. an array of bytes). This can be used to represent files as images or any other resources into memory.

- Array is implemented by means of array of values, which you can think it should be called _list_ instead. But as the memory layout is stored into a contiguous memory block of references to values, it is considered an array.

- Map implements an associative key value pair container. A map is implemented with an array of two sized elements array. Each element of the map is an array of size two, where the first element of it is always an String and the second element is a value of any type.

- Pointer is an opaque value representing a raw reference to a memory block. Some languages allow to use references to memory and some others not. This type is opaque because **METACALL** does not know what kind of concrete value represents it. The representation may be a complex type handled by the developer source code inside the run-time.

If any potential number overflow or invalid conversion between types is done, **METACALL** will warn about it. If any conversion of types can be handled by **METACALL**, it will automatically cast or transform the values into the target type automatically in order to avoid errors in the call.

#### 4.2.2 Values

#### 4.2.3 Functions

### 4.3 Loaders

#### 4.3.1 Python

#### 4.3.2 NodeJS

#### 4.3.3 JavaScript

#### 4.3.4 C# #

#### 4.3.5 Ruby

#### 4.3.6 Mock

#### 4.3.7 File

### 4.4 Ports

### 4.5 Serialization

### 4.6 Memory Layout

### 4.7 Fork Model

### 4.8 Threading Model

## 5. Platform Support
