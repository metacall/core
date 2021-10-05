# Go Port

[![PkgGoDev](https://pkg.go.dev/badge/github.com/metacall/core/source/ports/go_port/source)](https://pkg.go.dev/github.com/metacall/core/source/ports/go_port/source) [![Go Report Card](https://goreportcard.com/badge/github.com/metacall/core)](https://goreportcard.com/report/github.com/metacall/core)

This project implements a wrapper of MetaCall API for Go.

## Examples

```go
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

	scripts := []string{ "test.mock" }

	if err := metacall.LoadFromFile("mock", scripts); err != nil {
		fmt.Println(err)
		return
	}

	ret, err := metacall.Call("three_str", "e", "f", "g")

	if err != nil {
		fmt.Println(err)
		return
	}

	if str, ok := ret.(string); ok {
		fmt.Println(str)
	}
}
```
