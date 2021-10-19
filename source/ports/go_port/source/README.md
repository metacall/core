# MetaCall Go Port

[![PkgGoDev](https://pkg.go.dev/badge/github.com/metacall/core/source/ports/go_port/source)](https://pkg.go.dev/github.com/metacall/core/source/ports/go_port/source) [![Go Report Card](https://goreportcard.com/badge/github.com/metacall/core)](https://goreportcard.com/report/github.com/metacall/core)

This project implements a wrapper of MetaCall API for Go.

## Examples

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

## Building

[Build and install MetaCall from source](https://github.com/metacall/core/blob/develop/docs/README.md#6-build-system) or [install precompiled binaries](https://github.com/metacall/install#install). Then run:

```sh
go build
```

In case of using precompiled binaries (in Linux), when running any application using MetaCall, you must set the environment variable `LD_LIBRARY_PATH` pointing to the MetaCall library (a part of setting any other required environment variable related to MetaCall if needed). For example:

```sh
export LD_LIBRARY_PATH="/gnu/store/`ls /gnu/store/ | grep metacall | head -n 1`/lib"
```

## Testing

For running tests:

```sh
go test
```

## Benchmarks

For running tests:

```sh
go test -bench=.
```
