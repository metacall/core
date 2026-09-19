# MetaCall Go Port

[![PkgGoDev](https://pkg.go.dev/badge/github.com/metacall/core/source/ports/go_port/source)](https://pkg.go.dev/github.com/metacall/core/source/ports/go_port/source) [![Go Report Card](https://goreportcard.com/badge/github.com/metacall/core)](https://goreportcard.com/report/github.com/metacall/core)

This project implements a wrapper of MetaCall API for Go. We support all MetaCall types, see this README for more info about MetaCall project and its type system [MetaCall Core README](https://github.com/metacall/core/blob/develop/docs/README.md). Currently, Neither MetaCall support unsigned types nor the GO port.

## Examples
# Calling a TS function in GO
`test.ts`:
```ts
export function concat(left: string, right: string): string {
        return left + right;
}
```

`main.go`:
```go
package main

import (
	metacall "github.com/metacall/core/source/ports/go_port/source"
	"os"
	"fmt"
)

func main() {

	if err := metacall.Initialize(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	defer metacall.Destroy()

	scripts := []string{ "test.ts" }

	if err := metacall.LoadFromFile("ts", scripts); err != nil {
		fmt.Println(err)
		return
	}

	ret, err := metacall.Call("concat", "hello", "world")

	if err != nil {
		fmt.Println(err)
		return
	}

	if str, ok := ret.(string); ok {
		fmt.Println(str)
	}	
}
```
# Calling TS async function in GO
`test.ts`:
```ts
export async function sum(a: number, b: number): Promise<number> {
	return a + b;
}
```
`main.go`:
```go
package main

import (
	metacall "github.com/metacall/core/source/ports/go_port/source"
	"os"
	"fmt"
	"sync"
)

func main() {
	var sum int
	var wg sync.WaitGroup
	// WaitGroup is not necessary, the promise can be chained or pass value in ctx for executing res.Send(...)
	wg.Add(1)
	_, err := metacall.Await("sum",
		func(value interface{}, ctx interface{}) interface{} {
			log.Println("from resolve callback ", value)
			sum = value
			wg.Done()
			return nil
		},
		func(value interface{}, ctx interface{}) interface{} {
			log.Println("from reject callback ", value)
			sum = value
			wg.Done()
			return nil
		},
		nil,
		10, 20)

	if err != nil {
		fmt.Println(err)
		return
	}

	fmt.Println(sum)
}
```
# Calling Python script using config in GO
`example.py`:
```py
def appName():
    return "metacall"
```
`test.json`:
```json
{
	"language_id": "py",
	"path": "<path>",
	"scripts": [ "example.py" ]
}
```
`main.go`:
```go
package main

import (
	metacall "github.com/metacall/core/source/ports/go_port/source"
	"fmt"
)

func main() {
	if err := metacall.LoadFromConfig("test.json"); err != nil {
		fmt.Println(err)
		return
	}

	val, err := Call("appName")
	if err != nil {
		fmt.Println(err)
		return
	}

	strVal, ok := val.(string)
	if ok {
		fmt.Println(strVal)
		return
	}
}
```
# Calling MetaCall C package in GO
`main.go`:
```go
package main

import (
	metacall "github.com/metacall/core/source/ports/go_port/source"
	"fmt"
)

func main() {
	options := map[string]interface{}{
		"libs":                 []string{"<path>"},
		"headers":              []string{"<path>"},
		"include_search_paths": []string{"<path>"},
	}
	if err := LoadFromPackageEx("c", "metacall", options); err != nil {
		fmt.Println(err)
		return
	}

	val, err := Call("metacall_print_info")
	if err != nil {
		fmt.Println(err)
		return
	}

	str, ok := val.(string)
	if ok {
		fmt.Println(str)
		return
	}
}
```

## MetaCall APIs in GO

| API | GO implementation | Functionality |
|---|---|---|
| metacall_load_from_file | LoadFromFile | load from a local file |
| metacall_load_from_memory | LoadFromMemory | load scripts from memory |
| metacall_load_from_package | LoadFromPackage | load from a simple package, module, ..etc providing it exists on local device in standard path (using LD_LIBRARY_PATH or given full path to the function)|
| metacall_load_from_package_ex | LoadFromPackageEx | load from a complex package, module, ..etc providing it exists on local device in non standard path (custom include paths, multiple libs)|
| metacall_load_from_configuration | LoadFromConfig | parses a configuration block or file to dynamically resolve paths, target runtimes, and dependencies |
| metacall_execution_path | ExecutionPath | append or define search pathes of runtime libraries, headers, ...etc for MetaCall loaders |

## Debuging

GO port uses GO profiles (goroutine, heap, trace, cmdline, symbol) and symbolizers to monitor and locate any leaks or bugs during development. Profiles are exported to a localhost server, you can see them by using go pprof cli tool via command `go tool pprof http://localhost:6060/debug/pprof/name?debug=n` (change name with the profile, n with values >= 0) for more info about GO profiles visit [Profiling go with pprof](https://jvns.ca/blog/2017/09/24/profiling-go-with-pprof/). To enable profiles create a `.env` from `.env.example`. you can use GO sanitizers also to improve the debugging.

## Prerequisites

- GO: 1.17 or higher
- Python: 3.12 or higher (for testing with go test)
- C compiler for CGO: Clang (preferred) or GCC
- MetaCall Core
---
## Building

[Build and install MetaCall from source](https://github.com/metacall/core/blob/develop/docs/README.md#6-build-system) or [install precompiled binaries](https://github.com/metacall/install#install). Then run:

```sh
go build
```

In case of using precompiled binaries (in Linux):
 - When building, `go build` may require to set CGO environment variables pointing to MetaCall libraries in case of undefined C headers during compilation:
	```sh
	export CGO_CFLAGS="-I/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/include"
	export CGO_LDFLAGS="-L/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/lib"
	```

 - When running any application using MetaCall, you must set the environment variable `LD_LIBRARY_PATH` pointing to the MetaCall library (a part of setting any other required environment variable related to MetaCall if needed). For example:
	```sh
	export LD_LIBRARY_PATH="/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/lib"
	```

## Testing

For running tests:

```sh
go test
```

## Benchmarks

For running benchmarks:

```sh
go test -bench=.
```
