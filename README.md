
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
  - [2. Language Support (Backends)](#2-language-support-backends)
  - [3. Use Cases](#3-use-cases)
  - [4. Examples](#4-examples)
  - [5. Architecture](#5-architecture)
    - [5.1 Overview](#51-overview)
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
  - [6. Platform Support](#6-platform-support)
  - [7. License](#7-license)

<!-- /TOC -->

## 1. Motivation

The **METACALL** project started time ago when I was coding a [Game Engine for an MMORPG](https://bitbucket.org/parrastudios/argentum-online-c). My idea was to provide an interface to allow other programmers extend the Game Engine easily. By that time, I was finishing the university so I decide to do my [Final Thesis]() based on the plug-in system for my Game Engine. After some refination of the system, I came up with **METACALL** and other use cases for the tool. Currently we are using **METACALL** to build a cutting edge [FaaS](https://metacall.io/) (Function as a Service) based on this technique to provide high scalability of the functions among multiple cores or **Function Mesh** pattern, a new technique I have developed to interconnect transparently functions in a distributed system based on this library.

## 2. Language Support (Backends)

- Currently supported languages and run-times:

|                              Language                              |                                           Runtime                                            |      Version      | Tag  |
|:------------------------------------------------------------------:|:--------------------------------------------------------------------------------------------:|:-----------------:|:----:|
|                 [Python](https://www.python.org/)                  |                  [Python C API](https://docs.python.org/3/c-api/intro.html)                  | **>= 3.2 <= 3.6** |  py  |
|                   [NodeJS](https://nodejs.org/)                    |                          [N API](https://nodejs.org/api/n-api.html)                          |    **8.11.1**     | node |
| [JavaScript](https://developer.mozilla.org/bm/docs/Web/JavaScript) |                                    [V8](https://v8.dev/)                                     |    **5.1.117**    |  js  |
|                [C#](https://dotnet.microsoft.com/)                 | [NetCore](https://github.com/dotnet/docs/blob/master/docs/core/tutorials/netcore-hosting.md) |    **1.1.10**     |  cs  |
|                   [Ruby](https://ruby-lang.org/)                   |                  [Ruby C API](https://silverhammermba.github.io/emberb/c/)                   | **>= 2.1 <= 2.3** |  rb  |
|                [Mock](/source/loaders/mock_loader)                 |                                            **∅**                                             |     **0.1.0**     | mock |

- Languages and run-times under construction:

|                              Language                              |                                                                            Runtime                                                                            | Tag  |
|:------------------------------------------------------------------:|:-------------------------------------------------------------------------------------------------------------------------------------------------------------:|:----:|
|                   [Java](https://www.java.com/)                    |                                              [JNI](https://docs.oracle.com/javase/8/docs/technotes/guides/jni/)                                               | java |
|                 [C/C++](http://www.cplusplus.com/)                 | [Clang](https://clang.llvm.org/) - [LLVM](https://llvm.org/) - [libffi](http://sourceware.org/libffi/) / [libffcall](https://www.gnu.org/software/libffcall/) |  c   |
|                [File](/source/loaders/file_loader)                 |                                                                             **∅**                                                                             | file |
|                     [Go](https://golang.org/)                      |                                                                          Go Runtime                                                                           |  go  |
| [JavaScript](https://developer.mozilla.org/bm/docs/Web/JavaScript) |                            [SpiderMonkey](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference)                             | jsm  |

## 3. Use Cases

**METACALL** can be used for the following use cases:

- Interconnect different technologies in the same project. It allows to have heterogeneous teams of developers working over same project in an isolated way and using different programming languages at the same time.

- Embedding programming languages to existing softwares. Game Engines, 3D Editors like [Blender](https://www.blender.org/), among others can take benefit of **METACALL** and extend the core functionality with higher level programming languages (aka scripting).

- Function as a Service. **METACALL** can be used to implement efficient FaaS architectures. We are using it to implement our own [FaaS](https://metacall.io/) based on **Function Mesh** pattern and high performance function scalability thanks to this library.

- Source code migrations. **METACALL** can wrap large and legacy code-bases, and provide an agnostic way to work with the codebase into a new programming language. Eventually the code can be migrated by parts, without need of creating a new project or stopping the production environment. Incremental changes can be done, solving the migration easily and with less time and effort.

- Porting low level libraries to high level languages transparently. With **METACALL** you can get rid of extension APIs like Python C API or NodeJS N-API. You can call directly low level libraries from your high level languages without making a wrapper in C or C++ for it.

As you can see, there are plenty of uses. **METACALL** introduces a new model of programming which allows an high interoperability between technologies. If you find any other use case just let us know about it with a Pull Request and we will add it to the list.

## 4. Examples

## 5. Architecture

### 5.1 Overview

### 5.2 Reflect

The module that holds the representation of types, values and functions is called [`reflect`](/source/reflect) and it handles the abstraction of code loaded into **METACALL**.

**METACALL** uses reflection and introspection techniques to inspect the code loaded by the [`loaders`](/source/loaders) in order to interpret it and provide an higher abstraction of it. With this higher abstraction **METACALL** can easily inter-operate between languages transparently.

#### 5.2.1 Type System

**METACALL** implements an abstract type system which is a binary representation of the types supported by it. This means that **METACALL** can convert any type of a language to its own type system and back. Each loader is responsible of doing this conversions.

**METACALL** maintains most of the types of the languages but not all are supported. If new types are added they have to be implemented in the [`reflect`](/source/reflect) module and also in the [`loaders`](/source/loaders) and [`serials`](/source/serials) to fully support it.

| Type    | Value                                                              |
|:-------:|--------------------------------------------------------------------|
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

- Boolean is mostly represented by an integer value. There are languages that does not support it so it gets converted to a integer value in the memory layout.

- Integer and Floating Point values provide a complete abstraction to numerical types. Type sizes are preserved and the correct type is used when using any number. This depends on the internal implementation of the value by the run-time. Although there can be problems related to this. A `bignum` type from Ruby may overflow if it is too big when trying to convert it to a `float` type in C#.

- String is represented by ASCII encoding currently. Future versions will implement multiple encodings to be interoperable between other language encodings.

- Buffer represents a blob of raw memory (i.e. an array of bytes). This can be used to represent files as images or any other resources into memory.

- Array is implemented by means of array of values, which you can think it should be called _list_ instead. But as the memory layout is stored into a contiguous memory block of references to values, it is considered an array.

- Map implements an associative key value pair container. A map is implemented with an array of two sized elements array. Each element of the map is an array of size two, where the first element of it is always an String and the second element is a value of any type.

- Pointer is an opaque value representing a raw reference to a memory block. Some languages allow to use references to memory and some others not. This type is opaque because **METACALL** does not know what kind of concrete value represents it. The representation may be a complex type handled by the developer source code inside the run-time.

#### 5.2.2 Values

Values represent the instances of the **METACALL** type system.

The memory layout guarantees to fit at least the same size of the types into memory. This means if a boolean type can be represented with one bit inside a value of one byte size, maybe this value is stored in a bigger memory block and this fact is architecture and platform dependant.

When converting values between different types, if any potential number overflow or invalid conversion between types is done, **METACALL** will warn about it. If any conversion of types can be handled by **METACALL**, it will automatically cast or transform the values into the target type automatically in order to avoid errors in the call.

The value model is implemented by means of object pool. Each value is a reference to a memory block allocated from a memory pool (which can be injected into **METACALL**). The references can be passed by value, this means **METACALL** copies the reference value instead of the data which this reference is pointing to, like most run-times do when managing their own values.

Each created value must be destroyed manually. Otherwise it will lead to a memory leak. This fact only occurs when dealing with **METACALL** at C level. If **METACALL** is being used in an higher language through [`ports`](/source/ports), the developer does not have to care about memory management.

...

#### 5.2.3 Functions

...

The type deduction can be done at different levels. For example, it is possible to guess function types from the loaded code.

``` python
def multiply(a: int, b: int) -> int:
  return a * b
```

If this code is loaded, **METACALL** will be able to inspect the types and define the signature. Signature includes the names of the arguments, the types of those arguments if any, and the return type if any.

It may be possible that the function loaded into **METACALL** is duck typed. This means it does not have information about what types it supports and therefore they cannot be inspected statically.

``` python
def multiply(a, b):
  return a * b
```

At low level **METACALL** must always know the types to do the call. This types can be inferred statically or dynamically and this has implications over the call model.

In the first example, we can simply call the function without specifying the types.

``` c
metacall("multiply", 3, 4); // 12
```

As the signature is already know the literal values `3` and `4` can be converted into **METACALL** values automatically. Note that in this case, as literal values are provided, if we pass a double floating point, the memory representation of the value will be corrupted as there is no possible way to detect input values and cast them to the correct target values.

In the second example, the values are not know. If we use the same API to call the function, **METACALL** will not be able to call correctly the function as its types are not know. To allow calls to duck typed functions the developer must specify the value types he is passing to the function.

``` c
const enum metacall_value_id multiply_types[] =
{
  METACALL_INT, METACALL_INT
};

metacallt("multiply", multiply_types, 3, 4); // 12
```

This method allows to pass different value types to the same function. The following call would be valid too.

``` c
const enum metacall_value_id multiply_types[] =
{
  METACALL_DOUBLE, METACALL_DOUBLE
};

metacallt("multiply", multiply_types, 3.0, 4.0); // 12.0
```



### 5.3 Plugins

#### 5.3.1 Loaders

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

### 5.8 Threading Model

## 5. Application Programming Interface (API)

## 6. Platform Support

## 7. License
