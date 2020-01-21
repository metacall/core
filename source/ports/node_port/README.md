<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/master/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%; margin: 0 auto;" width="80" height="80">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**METACALL** is a library that allows calling functions, methods or procedures between programming languages. With **METACALL** you can transparently execute code from / to any programming language, for example, call Python code from NodeJS.

# Install

Install MetaCall binaries first:
```bash
curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | bash
```

Then install NodeJS package:
```bash
metacall install node metacall
```

# Example

`sum.py`
``` python
def sum(a, b):
  return a + b
```

`main.js`
``` javascript
const { metacall, metacall_load_from_file } = require('metacall');

metacall_load_from_file('py', [ 'sum.py' ]);

metacall('sum', 3, 4); // 7
```

``` sh
metacall load node main.js
```
