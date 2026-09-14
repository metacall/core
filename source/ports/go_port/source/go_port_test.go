package metacall

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"reflect"
	"sync"
	"testing"
	"unsafe"

	"github.com/joho/godotenv"
)

func TestMain(m *testing.M) {
	if err := Initialize(); err != nil {
		log.Fatal(err)
	}

	// if benchmark {
	buffer := "module.exports = { benchmark: async x => x }"

	if err := LoadFromMemory("node", buffer); err != nil {
		log.Fatal(err)
		return
	}
	// }

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

func TestNodeJSArray(t *testing.T) {
	buffer := "module.exports = { g: () => [0, 1, 2] }"

	if err := LoadFromMemory("node", buffer); err != nil {
		t.Fatal(err)
		return
	}

	array, err := Call("g")

	if err != nil {
		t.Fatal(err)
		return
	}

	iArray := array.([]interface{})

	if len(iArray) != 3 {
		t.Fatal("Invalid size of array")
		return
	}

	for i := 0; i < len(iArray); i++ {
		switch iArray[i].(type) {
		case float64:
			{
				if iArray[i] != float64(i) {
					t.Fatalf("Invalid value of array at position %d: %f (current) != %f (expected)", i, iArray[i], float64(i))
					return
				}
			}
		default:
			{
				t.Fatalf("Invalid type of array at position %d", i)
				return
			}
		}
	}
}

func TestNodeJSAwait(t *testing.T) {
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

func TestCPackage(t *testing.T) {
	options := map[string]interface{}{
		"libs":                 []string{"/mnt/Work/Projects/MetaCall/core/build/libmetacall.so"},
		"headers":              []string{"/mnt/Work/Projects/MetaCall/core/source/metacall/include/metacall/metacall.h"},
		"include_search_paths": []string{"/mnt/Work/Projects/MetaCall/core/source/metacall/include"},
	}
	if err := LoadFromPackageEx("c", "metacall", options); err != nil {
		t.Fatal(err)
	}

	val, err := Call("metacall_print_info")
	if err != nil {
		t.Fatal(err)
	}

	str, ok := val.(string)
	if !ok {
		t.Fatalf("failed to convert to string. got: %v", val)
	}
	t.Logf("test c package load success. got: %s", str)
}

func TestJSONConfig(t *testing.T) {
	configName := "test.json"
	scriptName := "example.py"

	jsonFile, err := os.Create(configName)
	if err != nil {
		t.Fatal(err)
	}

	defer jsonFile.Close()
	defer os.Remove(configName)

	pyFile, err := os.Create(scriptName)
	if err != nil {
		t.Fatal(err)
	}

	defer pyFile.Close()
	defer os.Remove(scriptName)

	scriptPath := "/mnt/Work/Projects/MetaCall/core/source/ports/go_port/source"

	config := fmt.Sprintf(`{
	"language_id": "py",
	"path": "%s",
	"scripts": [ "%s" ]
}
`, scriptPath, scriptName)

	// Create a temp example.py with functions to test
	scriptContent := `
def appName():
    return "metacall"
`
	if _, err := pyFile.WriteString(scriptContent); err != nil {
		t.Fatal(err)
	}

	if _, err = jsonFile.WriteString(config); err != nil {
		t.Fatal(err)
	}

	if err := LoadFromConfig(configName); err != nil {
		t.Fatal(err)
	}

	val, err := Call("appName")
	if err != nil {
		t.Fatal(err)
	}
	strVal, ok := val.(string)
	if !ok {
		t.Fatalf("failed to convert value %v to string. value is not a string", val)
	}

	if strVal != "metacall" {
		t.Fatalf("wrong config value. want: metacall ,got: %s", strVal)
	}
}

func TestExecutionPath(t *testing.T) {
	if err := ExecutionPath("c", "/mnt/Work/Projects/MetaCall/core/source/metacall/include"); err != nil {
		t.Fatal(err)
	}
	if err := ExecutionPath("c", "/mnt/Work/Projects/MetaCall/core/source/metacall/include/metacall"); err != nil {
		t.Fatal(err)
	}
	if err := ExecutionPath("c", "/mnt/Work/Projects/MetaCall/core/build"); err != nil {
		t.Fatal(err)
	}

	if err := LoadFromPackage("c", "metacall"); err != nil {
		t.Fatal(err)
	}

	val, err := Call("metacall_print_info")
	if err != nil {
		t.Fatal(err)
	}

	str, ok := val.(string)
	if !ok {
		t.Fatalf("failed to convert to string. got: %v", val)
	}
	t.Logf("test execution path success. got: %s", str)
}

func TestValues(t *testing.T) {
	// BitsPerWord is 32 or 64
	const BitsPerWord = 32 << (^uint(0) >> 63)

	// Calculate MIN and MAX based on BitsPerWord
	const MIN_LONG = int64(-1 << (BitsPerWord - 1))
	const MAX_LONG = int64(1<<(BitsPerWord-1) - 1)

	// Create pointer values
	var nullPtr *int = nil
	var bytePtr *byte = new(byte('H'))
	var intPtr *int = new(1)
	var floatPtr *float64 = new(1.5)

	tests := []struct {
		name  string
		input interface{}
		want  interface{}
	}{
		{"null", nil, nil},
		{"bool_true", true, true},
		{"bool_false", false, false},
		{"char", byte('H'), byte('H')},
		{"char_min", byte(127), byte(127)},
		{"char_max", byte(128), byte(128)},
		{"short", int16(1), int16(1)},
		{"short_min", int16(-32768), int16(-32768)},
		{"short_max", int16(32767), int16(32767)},
		{"int", int(1), int(1)},
		{"int32_min", int32(-2147483648), int(-2147483648)},
		{"int32_max", int32(2147483647), int(2147483647)},
		{"int_32sys_min", int(-2147483648), int(-2147483648)},
		{"int_32sys_max", int(2147483647), int(2147483647)},
		{"int_64sys_min", int(MIN_LONG), int64(MIN_LONG)},
		{"int_64sys_max", int(MAX_LONG), int64(MAX_LONG)},
		{"long", int64(3), int64(3)},
		{"long_min", MIN_LONG, MIN_LONG},
		{"long_max", MAX_LONG, MAX_LONG},
		{"float", float32(1.0), float32(1.0)},
		{"float_min", float32(1.2e-38), float32(1.2e-38)},
		{"float_max", float32(3.4e+38), float32(3.4e+38)},
		{"double", float64(1.0), float64(1.0)},
		{"double_min", float64(2.3e-308), float64(2.3e-308)},
		{"double_max", float64(1.7e+308), float64(1.7e+308)},
		{"string", "hello", "hello"},
		{"buffer_empty", *bytes.NewBuffer([]byte{}), *bytes.NewBuffer([]byte{})},
		{"buffer_nil", *bytes.NewBuffer(nil), *bytes.NewBuffer([]byte{})}, // TODO: how to handle nil buffer?
		{"buffer_ascii", *bytes.NewBuffer([]byte{'A', 'B', 'C'}), *bytes.NewBuffer([]byte{'A', 'B', 'C'})},
		{"buffer_unicode", *bytes.NewBuffer([]byte("\u00A9\u00A9\u00A9")), *bytes.NewBuffer([]byte("\u00A9\u00A9\u00A9"))},
		{"null_pointer", unsafe.Pointer(nullPtr), unsafe.Pointer(nullPtr)},
		{"byte_pointer", unsafe.Pointer(bytePtr), unsafe.Pointer(bytePtr)},
		{"int_pointer", unsafe.Pointer(intPtr), unsafe.Pointer(intPtr)},
		{"float_pointer", unsafe.Pointer(floatPtr), unsafe.Pointer(floatPtr)},
		{"exception", errors.New("test"), errors.New("Error : test")},
		{"array", [3]interface{}{1, 2, 3}, []interface{}{1, 2, 3}},
		{"array_bool", [3]bool{true, false, true}, []interface{}{true, false, true}},
		{"array_char", [3]byte{'1', '2', '3'}, []interface{}{byte('1'), byte('2'), byte('3')}},
		{"array_short", [3]int16{1, 2, 3}, []interface{}{int16(1), int16(2), int16(3)}},
		{"array_int", [3]int{1, 2, 3}, []interface{}{int(1), int(2), int(3)}},
		{"array_float", [3]float32{1.0, 2.0, 3.0}, []interface{}{float32(1.0), float32(2.0), float32(3.0)}},
		{"array_double", [3]float64{1.0, 2.0, 3.0}, []interface{}{float64(1.0), float64(2.0), float64(3.0)}},
		{"array_string", [3]string{"1", "2", "3"}, []interface{}{"1", "2", "3"}},
		{"slice", []interface{}{1, 2, 3}, []interface{}{1, 2, 3}},
		{"slice_bool", []bool{true, false, true}, []interface{}{true, false, true}},
		{"slice_char", []byte{'1', '2', '3'}, []interface{}{byte('1'), byte('2'), byte('3')}},
		{"slice_short", []int16{1, 2, 3}, []interface{}{int16(1), int16(2), int16(3)}},
		{"slice_int", []int{1, 2, 3}, []interface{}{int(1), int(2), int(3)}},
		{"slice_float", []float32{1.0, 2.0, 3.0}, []interface{}{float32(1.0), float32(2.0), float32(3.0)}},
		{"slice_double", []float64{1.0, 2.0, 3.0}, []interface{}{float64(1.0), float64(2.0), float64(3.0)}},
		{"slice_string", []string{"1", "2", "3"}, []interface{}{"1", "2", "3"}},
		{"map", map[string]interface{}{"1": 1, "2": true, "3": "sss", "4": float32(1.1), "5": byte('A')}, map[string]interface{}{"1": 1, "2": true, "3": "sss", "4": float32(1.1), "5": byte('A')}},
		{"map_bool", map[string]bool{"1": true, "2": false, "3": true}, map[string]interface{}{"1": true, "2": false, "3": true}},
		{"map_char", map[string]byte{"1": 'A', "2": 'B', "3": 'C'}, map[string]interface{}{"1": byte('A'), "2": byte('B'), "3": byte('C')}},
		{"map_short", map[string]int16{"1": 1, "2": 2, "3": 3}, map[string]interface{}{"1": int16(1), "2": int16(2), "3": int16(3)}},
		{"map_int", map[string]int{"1": 1, "2": 2, "3": 3}, map[string]interface{}{"1": int(1), "2": int(2), "3": int(3)}},
		{"map_float", map[string]float32{"1": 1.0, "2": 2.0, "3": 3.0}, map[string]interface{}{"1": float32(1.0), "2": float32(2.0), "3": float32(3.0)}},
		{"map_double", map[string]float64{"1": 1.0, "2": 2.0, "3": 3.0}, map[string]interface{}{"1": float64(1.0), "2": float64(2.0), "3": float64(3.0)}},
		{"map_string", map[string]string{"1": "1", "2": "2", "3": "3"}, map[string]interface{}{"1": "1", "2": "2", "3": "3"}},
	}

	for _, tt := range tests {
		var ptr unsafe.Pointer
		goToValue(tt.input, &ptr)

		if ptr == nil {
			t.Errorf("invalid pointer: %s", tt.name)
			return
		}

		if v := valueToGo(ptr); !reflect.DeepEqual(v, tt.want) {
			t.Errorf("name: %s, input: %T,%v, want: %T,%v, got: %T,%v", tt.name, tt.input, tt.input, tt.want, tt.want, v, v)
		}
	}
}

func TestPythonClassAndObject(t *testing.T) {
	script := `class Rectangle:
    color = "blue"
    width = 0
    height = 0

    def __init__(self, width, height):
        self.width = width
        self.height = height

    def area(self):
        return self.width * self.height

def getClass():
    return Rectangle
`

	if err := LoadFromMemory("py", script); err != nil {
		t.Fatalf("failed to load script: %v", err)
	}

	val, err := Call("getClass")
	if err != nil {
		t.Fatalf("failed to get class: %v", err)
	}

	class, ok := val.(*Class)
	if !ok || class == nil {
		t.Fatalf("expected *Class, got %T", val)
	}

	color, err := class.StaticGet("color")
	if err != nil || color != "blue" {
		t.Fatalf("expected static color 'blue', got %v (err: %v)", color, err)
	}

	if err := class.StaticSet("color", "red"); err != nil {
		t.Fatalf("failed to set static attribute: %v", err)
	}

	newColor, err := class.StaticGet("color")
	if err != nil || newColor != "red" {
		t.Fatalf("expected updated color 'red', got %v (err: %v)", newColor, err)
	}

	obj, err := class.New("rectInstance", 10, 20)
	if err != nil || obj == nil {
		t.Fatalf("failed to create object instance: %v", err)
	}

	area, err := obj.Call("area")
	if err != nil {
		t.Fatalf("failed to call object method 'area': %v", err)
	}
	if area != 200 && area != int64(200) {
		t.Fatalf("expected area 200, got %v", area)
	}

	width, err := obj.Get("width")
	if err != nil || (width != 10 && width != int64(10)) {
		t.Fatalf("expected width 10, got %v (err: %v)", width, err)
	}

	if err := obj.Set("width", 15); err != nil {
		t.Fatalf("failed to set width: %v", err)
	}

	newArea, err := obj.Call("area")
	if err != nil {
		t.Fatalf("failed to call 'area' after update: %v", err)
	}
	if newArea != 300 && newArea != int64(300) {
		t.Fatalf("expected updated area 300, got %v", newArea)
	}
}

func TestNodeJSFuture(t *testing.T) {
	script := `                                                                                                                  
        module.exports = {                                                                                                           
            asyncAdd: async (a, b) => {                                                                                                  
                return a + b;                                                                                                                
            },                                                                                                                           
            asyncFail: async (msg) => {                                                                                                  
                throw new Error(msg);                                                                                                        
            },                                                                                                                           
        };                                                                                                                           
        `

	if err := LoadFromMemory("node", script); err != nil {
		t.Fatalf("failed to load script: %v", err)
	}

	val, err := Call("asyncAdd", 20, 30)
	if err != nil {
		t.Fatalf("call to asyncAdd failed: %v", err)
	}

	fut, ok := val.(*Future)
	if !ok || fut == nil {
		t.Fatalf("expected *Future, got %T", val)
	}

	res, err := fut.Await()
	if err != nil {
		t.Fatalf("await returned unexpected error: %v", err)
	}
	if res != 50 && res != float64(50) {
		t.Fatalf("expected 50, got %v", res)
	}

	failVal, err := Await("asyncFail",
		func(interface{}, interface{}) interface{} {
			log.Println("from go resolve")
			return nil
		},
		func(interface{}, interface{}) interface{} {
			log.Println("from go reject")
			return nil
		},
		"database error")
	if err != nil {
		t.Fatalf("call to asyncFail failed: %v", err)
	}

	failFut, ok := failVal.(*Future)
	if !ok || failFut == nil {
		t.Fatalf("expected *Future, got %T", failVal)
	}

	_, awaitErr := failFut.Await()
	if awaitErr == nil {
		t.Fatal("expected error from rejected future, got nil")
	}
}

func TestProfilesServer(t *testing.T) {
	err := godotenv.Load()
	if err == nil {
		mode := os.Getenv("MODE")
		if mode != "debug" {
			return
		}
	}
	// use http instead of curl for cross-platform support
	c := &http.Client{}
	reqs := [3][2]string{
		{"goroutine", "http://localhost:6060/debug/pprof/goroutine?debug=0"},
		{"memory", "http://localhost:6060/debug/pprof/heap?debug=0"},
		{"trace", "http://localhost:6060/debug/pprof/trace?debug=0"},
	}

	for _, req := range reqs {
		profileReq, err := http.NewRequest("GET", req[1], nil)
		if err != nil {
			t.Fatal(err)
		}

		profileReq.Header.Set("Accept", "application/json")
		profileRes, err := c.Do(profileReq)

		if err != nil {
			t.Fatal(err)
		}
		defer profileRes.Body.Close()

		if profileRes.StatusCode != http.StatusOK {
			t.Fatalf("profile server test response failed with status code %v", profileRes.StatusCode)
		}
		data, err := io.ReadAll(profileRes.Body)
		if err != nil {
			t.Fatal(err)
		}
		// create .pprof file for go tool pprof
		profFile, err := os.Create(req[0] + ".pprof")
		if err != nil {
			t.Fatal(err)
		}
		defer profFile.Close()

		_, err = profFile.Write(data)
		if err != nil {
			t.Fatal(err)
		}
	}
}

func benchmarkNodeJS(b *testing.B, n int) {
	var wg sync.WaitGroup

	wg.Add(n)

	for i := 0; i < n; i++ {
		_, err := Await("benchmark",
			func(interface{}, interface{}) interface{} {
				wg.Done()
				return nil
			},
			func(interface{}, interface{}) interface{} {
				wg.Done()
				return nil
			},
			nil,
		)

		if err != nil {
			b.Fatal(err)
			return
		}
	}

	wg.Wait()
}

func BenchmarkNodeJSSequential(b *testing.B) {
	benchmarkNodeJS(b, 5)
}

func BenchmarkNodeJSParallel(b *testing.B) {
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			benchmarkNodeJS(b, 5)
		}
	})
}
