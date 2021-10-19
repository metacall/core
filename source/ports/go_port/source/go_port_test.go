package metacall

import (
	"log"
	"os"
	"sync"
	"testing"
)

func TestMain(m *testing.M) {
	if err := Initialize(); err != nil {
		log.Fatal(err)
	}
	code := m.Run()
	Destroy()
	os.Exit(code)
}

func TestMock(t *testing.T) {
	scripts := []string{"test.mock"}

	if err := LoadFromFile("mock", scripts); err != nil {
		t.Fatal(err)
		return
	}

	ret, err := Call("three_str", "e", "f", "g")

	if err != nil {
		t.Fatal(err)
		return
	}

	str, ok := ret.(string)

	if !ok || str != "Hello World" {
		t.Fatalf("expected 'Hello World', received %s", str)
	}
}

func TestNodeJS(t *testing.T) {
	buffer := "module.exports = { f: async (ms) => await new Promise(resolve => setTimeout(resolve, ms)) }"

	if err := LoadFromMemory("node", buffer); err != nil {
		t.Fatal(err)
		return
	}

	var wg sync.WaitGroup

	wg.Add(1)

	_, err := Await("f",
		func(interface{}, interface{}) interface{} {
			log.Println("from go resolve")
			wg.Done()
			return nil
		},
		func(interface{}, interface{}) interface{} {
			log.Println("from go reject")
			wg.Done()
			return nil
		},
		nil,
		300, // 0.3 seconds
	)

	if err != nil {
		t.Fatal(err)
		return
	}

	wg.Wait()
}
