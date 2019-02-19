
<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="/deploy/images/logo.png" alt="M E T A C A L L" style="max-width:100%;" width="32" height="32">
  <p><b>M E T A C A L L</b></p></a>
  <p>A library for providing inter-language foreign function interface calls</p>
</div>

# Abstract

**METACALL** Python Port is the port of **METACALL** to Python Programming Language.
With **METACALL** Python Port you can transparently execute code from Python to any programming language, for example, calling JavaScript (NodeJS) code from Python.


`sum.js`
``` javascript
module.exports = function sum(a, b) {
    return a + b;
};
```

`main.py`
``` python
from metacall import metacall_load_from_file, metacall

metacall_load_from_file('node', [ 'sum.js' ]);

metacall('sum', 3, 4); // 7
```
