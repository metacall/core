package main

import "github.com/metacall/core/source/ports/go_port/source"

func main() {

	if err := metacall_initialize(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	defer metacall_destroy()

	scripts := []string{ "test.mock" }

	if err := metacall_load_from_file("mock", scripts); err != nil {
		fmt.Println(err)
		return
	}

	ret, err := metacall("three_str", "e", "f", "g")

	if err != nil {
		fmt.Println(err)
		return
	}

	if str, ok := ret.(string); ok {
		fmt.Println(str)
	}
}
