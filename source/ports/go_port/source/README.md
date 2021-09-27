# Go Port

This project implements a wrapper of MetaCall API for Go.

## Examples

Non-thread safe:
```go
import (
	"github.com/metacall/core/source/ports/go_port/source"
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

Thread safe:
```go
import (
	"github.com/metacall/core/source/ports/go_port/source"
	"os"
)

func main() {

	if err := metacall.InitializeSafe(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	defer metacall.DestroySafe()

	scripts := []string{"test.mock"}

	if err := metacall.LoadFromFileSafe("mock", scripts); err != nil {
		fmt.Println(err)
		return
	}

	ret, err := metacall.CallSafe("three_str", "e", "f", "g")

	if err != nil {
		fmt.Println(err)
		return
	}

	if str, ok := ret.(string); ok {
		fmt.Println(str)
	}
}
```
