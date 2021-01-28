# MetaCall Scala Port

## Setup

To set up Scala & SBT, use [Coursier](https://get-coursier.io/docs/cli-installation). After getting the `cs` executable, run `cs setup` and follow the prompt.

## Testing

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

> Note: You'll find the bindings and the code that runs on `sbt test` in `src/main/scala/MetaCall.scala`.

## Debugging

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
