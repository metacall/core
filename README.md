<div align="center">
  <a href="https://metacall.io" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/develop/deploy/images/logo.png" alt="METACALL" style="max-width:100%; margin: 0 auto;" width="80" height="80"></a>
  <h1><b>MetaCall Polyglot Runtime</b></h1>
  <a href="https://metacall.io">MetaCall.io</a> |
  <a href="#install">Install</a> |
  <a href="https://github.com/metacall/core/blob/develop/docs/README.md">Docs</a>
</div>
<div id="badges" align="center">
  <a href="https://t.me/joinchat/BMSVbBatp0Vi4s5l4VgUgg" alt="Discord">
    <img src="https://img.shields.io/static/v1?label=metacall&message=join&color=blue&logo=telegram&style=flat" /></a>

  <a href="https://discord.gg/upwP4mwJWa" alt="Discord">
    <img src="https://img.shields.io/discord/781987805974757426?label=discord&style=flat" /></a>

  <a href="https://matrix.to/#/#metacall:matrix.org" alt="Matrix">
    <img src="https://img.shields.io/matrix/metacall:matrix.org?label=matrix&style=flat" /></a>

  <a href="https://twitter.com/metacallio" alt="Twitter">
    <img src="https://img.shields.io/twitter/follow/metacallio?label=MetaCall" /></a>

  <a href="https://medium.com/@metacall/call-functions-methods-or-procedures-between-programming-languages-with-metacall-58cfece35d7" target="_blank"><img src="https://raw.githubusercontent.com/metacall/core/develop/deploy/images/overview.png" alt="M E T A C A L L" style="max-width:100%; margin: 0 auto;" width="350" height="auto"></a>
</div>

**MetaCall** allows calling functions, methods or procedures between multiple programming languages.

`sum.py`
``` python
def sum(a, b):
  return a + b
```

`main.js`
``` javascript
const { sum } = require('./sum.py');

sum(3, 4); // 7
```

`shell`
``` sh
metacall main.js
```

**MetaCall** is a extensible, embeddable and interoperable cross-platform polyglot runtime. It supports NodeJS, Vanilla JavaScript, TypeScript, Python, Ruby, C#, Java, WASM, Go, C, C++, Rust, D, Cobol [and more](https://github.com/metacall/core/blob/develop/docs/README.md#2-language-support).

## Install

The easiest way to install **MetaCall** is the following:

Under ArchLinux (replace `aura` by your AUR helper):
```
sudo aura -Axc metacall-git
```

You should also set the following environment variables to `on` or `off` in order to build support for specific loaders :
- METACALL_LOADER_C
- METACALL_LOADER_CS
- METACALL_LOADER_COB
- METACALL_LOADER_JAVA
- METACALL_LOADER_NODE
- METACALL_LOADER_JS
- METACALL_LOADER_PY
- METACALL_LOADER_RB
- METACALL_LOADER_TS



``` sh
curl -sL https://raw.githubusercontent.com/metacall/install/master/install.sh | sh
```

For more information about other install methodologies and platforms or Docker, check the [install documentation](https://github.com/metacall/core/blob/develop/docs/README.md#41-installation).

## Examples

You can find a complete [list of examples in the documentation](https://github.com/metacall/core/blob/develop/docs/README.md#43-examples). If you are interested in submitting new examples, please [contact us in our chats](#badges).
