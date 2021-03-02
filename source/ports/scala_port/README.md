# MetaCall Scala Port

A library for calling NodeJS, Python, and Ruby functions from Scala.

```js
// myfunctions.js

function hello(x) {
    return 'Hello, ' + x
}

module.exports = { hello }
```
```scala
// Main.scala

import metacall._, instances._
import java.nio.file.Paths
import scala.concurrent.{Future, Await, ExecutionContext}
import scala.concurrent.duration._
import ExecutionContext.Implicits.global

object Main extends App {
  Caller.start(ExecutionContext.global)

  val future: Future[Value] = for {
    _      <- Caller.loadFile(Runtime.Node, Paths.get("./myfunctions.js").toAbsolutePath.toString)
    result <- Caller.call("hello", "World!")
  } yield result

  println(Await.result(future, 1.second))
  // metacall.StringValue("Hello, World!")

  Caller.destroy()
}
```

## Usage

To import most of MetaCall's functionality, use:
```scala
import metacall._, instances._
```
This imports the important types and type class instances. There are a few important types to note:
- `Value`: A coproduct representing the types of values that can be passed as arguments to and returned from foreign functions.
- `Args[A]`: A type class that allows product types (e.g. `case class`es, tuples, `shapeless.HList`s) to be passed as arguments to foreign functions. Instances for this type class are defined in `metacall.instances`.

The `Caller` object defines methods for calling the foreigh functions, such as `call`, and `callV`. **Before doing anything, remember that you need to call `Caller.start()`; you should also call `Caller.destroy()` when you're done using the `Caller`. This should be done only once during the life of the application.**
```scala
Caller.start(concurrent.ExecutionContext.global)
// scala.util.Try[Unit]

// Takes a product as arguments (A: Args)
Caller.call("fnName", ("arg1", "arg2"))
// scala.concurrent.Future[metacall.Value]

// Takes a List[Value] as arguments
Caller.callV("fnName", List(StringValue("arg1"), StringValue("arg2")))
// scala.concurrent.Future[metacall.Value]

Caller.destroy()
// scala.util.Try[Unit]
```

> Calling foreign functions always returns a `Future`, even if the foreign function isn't `async`. If the function is `async`, or it returns a promise, its return value would be flattened into a `Future[Value]`.

Functions need to be loaded before they are called; for that, we have the `loadFile` and `loadFiles` methods.
```scala
Caller.loadFile(Runtime.Node, Paths.get("./myfunctions.js").toAbsolutePath.toString)
// scala.concurrent.Future[Unit]
```
Optionally, you can specify a namespace where you want the functions to be loaded.
```scala
import java.nio.file.Paths

Caller.loadFile(
  Runtime.Node, 
  Paths.get("./myfunctions.js").toAbsolutePath.toString,
  Some("myNamespace")
)
// scala.concurrent.Future[Unit]
```
Then you can pass the namespace to `call` or `callV` to call functions from it. You can also see all the definitions in a namespace using the `definitions` method.

## Development
### Setup

To set up Scala & SBT, use [Coursier](https://get-coursier.io/docs/cli-installation). After getting the `cs` executable, run `cs setup` and follow the prompt.

Compiling requires setting either the `GITHUB_TOKEN` environment variable, or a `github.token` Git configuration. Use `export GITHUB_TOKEN=<token>` or `git config --global github.token <token>`, where `<token>` can be generated in your GitHub account's [settings](https://github.com/settings/tokens).

### Testing

To run the tests, run `sbt test` in this README's directory.

Don't forget to set these environment variables:
```
LOADER_SCRIPT_PATH
LOADER_LIBRARY_PATH
CONFIGURATION_PATH
SERIAL_LIBRARY_PATH
DETOUR_LIBRARY_PATH
PORT_LIBRARY_PATH
```

These variables are set automatically if MetaCall is intalled from source, i.e. using `sudo make install`:
```sh
cd ./core
mkdir build && cd build
cmake .. # Use loader flags as specified in https://github.com/metacall/core/blob/develop/docs/README.md#6-build-system
sudo make install
```

> You need to set `LOADER_LIBRARY_PATH` to the build directory created in the script above before running `sbt`, i.e. `LOADER_LIBRARY_PATH=path/to/core/build sbt`

To run the tests in Docker, run `sbt` then `docker` to build the image (must run `docker` from within the SBT session), and then `sbt dockerTest` to run it. Note that you should build the `metacall/core:dev` image locally since the published one might not be up to date by running `./docker-compose.sh build` in `metacall/core`'s root. Pay attention to SBT's error messages.

### Debugging

Uncomment this line in `build.sbt`:
```
"-Djava.compiler=NONE",
```

Build the project:
```
sbt compile
```

For runing valgrind with the correct classpath, run:
```
sbt "export test:fullClasspath"
```

Then copy the classpath into the valgrind command:
```
valgrind --tool=memcheck --trace-children=yes --error-limit=no scala -Djava.compiler=NONE -cp <classpath> src/test/scala/MetaCallSpecMain.scala
```

# Publishing

Use `sbt publish` to publish to GitHub Packages using [sbt-github-packages](https://github.com/djspiewak/sbt-github-packages). Make sure your GitHub token is set correctly according to [Setup](#setup).