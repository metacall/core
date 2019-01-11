
<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/develop/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%;" width="32" height="32">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**METACALL Ports** is a complete infrastructure for supporting multiple language bindings in MetaCall library.

# Table Of Contents

<!-- TOC -->

- [Abstract](#abstract)
- [Table Of Contents](#table-of-contents)
  - [1. Build System](#1-build-system)
  - [2. License](#2-license)

<!-- /TOC -->

## 1. Build System

Follow these steps to build and install **METACALL** manually.

``` sh
git clone --recursive https://github.com/metacall/ports.git
mkdir ports/build && cd ports/build
cmake ..
make
make test
sudo make install
```

## 2. License

**METACALL Ports** is licensed under **[Apache License Version 2.0](/LICENSE)**.

>[Copyright](/COPYRIGHT) (C) 2016 - 2019 Vicente Eduardo Ferrer Garcia <<vic798@gmail.com>>
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
