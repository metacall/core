# C++ CMake Project Template

cmake-init is a copy and paste template, that provides the following features:
 * Cross Platform
  * Windows
  * Linux
  * Mac
 * Libraries, Applications, Testing template
 * Documentation template
 * Installation, Packaging template
 * CMake find script template for defined libraries

The files of cmake-init are an instantiation of the templates containing:
  * Example app
  * Example lib
  * Example test
  * Example documentation
  * Example package

| Service | System | Compiler | Status |
| ------- | ------ | -------- | -----: |
|  [Travis-CI](https://travis-ci.org/cginternals/cmake-init) | Ubuntu 14.04 | GCC 4.8, Clang 3.5 | [![Build Status](https://travis-ci.org/cginternals/cmake-init.svg?branch=master)](https://travis-ci.org/cginternals/cmake-init) |
| Jenkins <br><br><br><br> | Ubuntu 14.04 <br><br><br><br> | GCC 4.7 <br> GCC 4.8 <br> GCC 4.9 <br> GCC 5.3 <br> Clang 3.5 <br> | [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-linux-gcc4.7)](http://jenkins.hpi3d.de/job/cmake-init-linux-gcc4.7) <br> [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-linux-gcc4.8)](http://jenkins.hpi3d.de/job/cmake-init-linux-gcc4.8) <br> [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-linux-gcc4.9)](http://jenkins.hpi3d.de/job/cmake-init-linux-gcc4.9) <br> [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-linux-gcc5.3)](http://jenkins.hpi3d.de/job/cmake-init-linux-gcc5.3) <br> [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-linux-clang3.5)](http://jenkins.hpi3d.de/job/cmake-init-linux-clang3.5) <br> |
| Jenkins <br><br> | Windows 10 <br><br> | MSVC 2013 Update 5 <br>  MSVC 2015 Update 1 <br> | [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-windows-msvc2013)](http://jenkins.hpi3d.de/job/cmake-init-windows-msvc2013) <br> [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-windows-msvc2015)](http://jenkins.hpi3d.de/job/cmake-init-windows-msvc2015) <br> |
| Jenkins | OS X 10.10 | AppleClang 6.0 | [![Build Status](http://jenkins.hpi3d.de/buildStatus/icon?job=cmake-init-osx-clang3.5)](http://jenkins.hpi3d.de/job/cmake-init-osx-clang3.5) |

Please note that our OS X build node is currently broken (physically). However, *cmake-init* is supported and maintained for OS X as well.


## Design Decisions

ToDo: revisit!

#### Console vs. Windows App (Windows only)

decisions:
* we do not use the msvc subsystem linker flag
* we do not use the add_executable win32 property
* instead the ```set_target_properties``` with the ```WIN32_EXECUTABLE``` flag on target executable is used
* we use the same subsystem for all configurations of a single target (no switching between, e.g., debug and release)

rationale:
typically, only released 'feature' applications are required to decide on their subsystem, and in the case of 'gui-heavy' applications the windows subsystem is preferred (since the std output is probably piped into widgets, logfiles, or even discard).

furthermore, console output is usually more relevant for development/debugging purposes, already using console or IDE with appropriate console output handling



ToDo: Add missing content, bring into markdown form


Scenarios covered in cmake-init:


1) Development

The project is contained in a source directory for active development.

binaries: ./build
rpath:    absolute paths to all dependencies
datapath: ..


2) Installation (default)

The project is installed in a self-contained directory, ready for being moved or copied to another location or computer.

binaries: ./bin
rpath:    $ORIGIN/../lib
datapath: ..


3) Installation (unix system install)

The project is installed globally on a system.

binaries: /usr/[local/]bin
rpath:    empty
datapath: /usr/[local/]share/<projectname>
