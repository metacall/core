package metacall

import (
	"bytes"
	"errors"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"reflect"
	"runtime"
	"runtime/pprof"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"
	"unsafe"
)

func TestMain(m *testing.M) {
	if err := Initialize(); err != nil {
		log.Fatal(err)
	}

	// If benchmark {
	if loaderEnabled("node") {
		buffer := "module.exports = { benchmark: async x => x }"

		if err := LoadFromMemory("node", buffer); err != nil {
			log.Fatal(err)
			return
		}
	}
	// }

	code := m.Run()
	Destroy()

	// Store the goroutines that are still alive after the shutdown
	if err := writeProfile("goroutine", "goroutine.pprof"); err != nil {
		log.Println(err)
		code = 1
	}

	leakCheck()
	os.Exit(code)
}

// Write a profile in binary format into PPROFDIR (or the current directory if it is not defined)
func writeProfile(name string, fileName string) error {
	prof := pprof.Lookup(name)
	if prof == nil {
		return fmt.Errorf("profile %s not found", name)
	}

	filePath := os.Getenv("PPROFDIR")
	if filePath == "" {
		filePath, _ = os.Getwd()
	}

	file, err := os.Create(filepath.Join(filePath, fileName))
	if err != nil {
		return err
	}
	defer file.Close()

	return prof.WriteTo(file, 0)
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

func loaderEnabled(tag string) bool {
	return os.Getenv("OPTION_BUILD_LOADERS_"+strings.ToUpper(tag)) == "ON"
}

func requireLoader(tb testing.TB, tag string) {
	if !loaderEnabled(tag) {
		tb.Skipf("loader %s is not enabled in this build", tag)
	}
}

func TestNodeJSArray(t *testing.T) {
	requireLoader(t, "node")
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
	requireLoader(t, "node")
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

func testEnv(t *testing.T, name string) string {
	value := os.Getenv(name)
	if value == "" {
		t.Fatalf("environment variable %s is not defined", name)
	}
	return value
}

func TestCPackage(t *testing.T) {
	requireLoader(t, "c")
	includeDir := testEnv(t, "METACALL_INCLUDE_DIR")
	options := map[string]interface{}{
		"libs":                 []string{testEnv(t, "METACALL_LIBRARY")},
		"headers":              []string{includeDir + "/metacall/metacall.h"},
		"include_search_paths": []string{includeDir, testEnv(t, "METACALL_API_INCLUDE_DIR"), testEnv(t, "METACALL_BUILD_INCLUDE_DIR")},
	}
	if err := LoadFromPackageEx("c", "metacall", options); err != nil && !strings.Contains(err.Error(), "already loaded") {
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
	requireLoader(t, "py")
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

	// The script is created in the working directory
	scriptPath, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}

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
	requireLoader(t, "c")
	includeDir := testEnv(t, "METACALL_INCLUDE_DIR")
	if err := ExecutionPath("c", includeDir); err != nil && !strings.Contains(err.Error(), "already loaded") {
		t.Fatal(err)
	}
	if err := ExecutionPath("c", includeDir+"/metacall"); err != nil && !strings.Contains(err.Error(), "already loaded") {
		t.Fatal(err)
	}
	if err := ExecutionPath("c", testEnv(t, "PROJECT_OUTPUT_DIR")); err != nil && !strings.Contains(err.Error(), "already loaded") {
		t.Fatal(err)
	}

	if err := LoadFromPackage("c", "metacall"); err != nil && !strings.Contains(err.Error(), "failed to load from package") && !strings.Contains(err.Error(), "already loaded") {
		t.Fatal(err)
	}

	val, err := Call("metacall_print_info")
	if err != nil {
		t.Fatal(err)
	}

	_, ok := val.(string)
	if !ok {
		t.Fatalf("failed to convert to string. got: %v", val)
	}
}

func TestValues(t *testing.T) {
	// BitsPerWord is 32 or 64
	const BitsPerWord = 32 << (^uint(0) >> 63)

	// Calculate MIN and MAX based on BitsPerWord
	const MIN_LONG = int64(-1 << (BitsPerWord - 1))
	const MAX_LONG = int64(1<<(BitsPerWord-1) - 1)

	// In 32-bit systems, int fits in metacall int, so it is returned as int
	var INT_SYS_MIN, INT_SYS_MAX interface{} = int64(MIN_LONG), int64(MAX_LONG)

	if BitsPerWord == 32 {
		INT_SYS_MIN, INT_SYS_MAX = int(MIN_LONG), int(MAX_LONG)
	}

	// Create pointer values
	var nullPtr *int = nil
	b := byte('H')
	var bytePtr *byte = &b
	i := 1
	var intPtr *int = &i
	f := 1.5
	var floatPtr *float64 = &f

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
		{"int_64sys_min", int(MIN_LONG), INT_SYS_MIN},
		{"int_64sys_max", int(MAX_LONG), INT_SYS_MAX},
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

		valueDestroy(ptr)
	}
}

func TestPythonClassAndObject(t *testing.T) {
	requireLoader(t, "py")
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

// Run the garbage collector so the finalizers are called
func runFinalizers() {
	for i := 0; i < 3; i++ {
		runtime.GC()
		time.Sleep(10 * time.Millisecond)
	}
}

func TestPythonOwnershipNested(t *testing.T) {
	requireLoader(t, "py")
	script := `class OwnershipPoint:
    x = 0

    def __init__(self, x):
        self.x = x

def ownership_nested():
    return [OwnershipPoint(1), {'p': OwnershipPoint(2)}, OwnershipPoint]
`
	if err := LoadFromMemory("py", script); err != nil {
		t.Fatalf("failed to load script: %v", err)
	}

	val, err := Call("ownership_nested")
	if err != nil {
		t.Fatalf("call to ownership_nested failed: %v", err)
	}

	array, ok := val.([]interface{})
	if !ok || len(array) != 3 {
		t.Fatalf("expected array of 3 elements, got %T: %v", val, val)
	}

	// Object inside of an array, the array value is already destroyed at this point
	obj, ok := array[0].(*Object)
	if !ok {
		t.Fatalf("expected *Object, got %T", array[0])
	}
	if x, err := obj.Get("x"); err != nil || fmt.Sprint(x) != "1" {
		t.Fatalf("expected x = 1, got %v (err: %v)", x, err)
	}

	// Object inside of a map
	m, ok := array[1].(map[string]interface{})
	if !ok {
		t.Fatalf("expected map, got %T", array[1])
	}
	mObj, ok := m["p"].(*Object)
	if !ok {
		t.Fatalf("expected *Object, got %T", m["p"])
	}
	if x, err := mObj.Get("x"); err != nil || fmt.Sprint(x) != "2" {
		t.Fatalf("expected x = 2, got %v (err: %v)", x, err)
	}

	// Class inside of an array
	cls, ok := array[2].(*Class)
	if !ok {
		t.Fatalf("expected *Class, got %T", array[2])
	}
	newObj, err := cls.New("OwnershipPoint", 3)
	if err != nil {
		t.Fatalf("failed to create object: %v", err)
	}
	if x, err := newObj.Get("x"); err != nil || fmt.Sprint(x) != "3" {
		t.Fatalf("expected x = 3, got %v (err: %v)", x, err)
	}

	array, m, obj, mObj, cls, newObj = nil, nil, nil, nil, nil, nil
	runFinalizers()
}

func TestPythonOwnershipAwaitObject(t *testing.T) {
	requireLoader(t, "py")
	script := `class OwnershipAsyncPoint:
    x = 0

    def __init__(self, x):
        self.x = x

async def ownership_make_point(x):
    return OwnershipAsyncPoint(x)
`
	if err := LoadFromMemory("py", script); err != nil {
		t.Fatalf("failed to load script: %v", err)
	}

	result := make(chan interface{}, 1)

	_, err := Await("ownership_make_point",
		func(v, _ interface{}) interface{} {
			// The value is owned by the loader, the object must outlive the callback
			result <- v
			return nil
		},
		func(v, _ interface{}) interface{} {
			result <- fmt.Errorf("rejected: %v", v)
			return nil
		},
		nil, 4)
	if err != nil {
		t.Fatalf("await of ownership_make_point failed: %v", err)
	}

	var v interface{}
	select {
	case v = <-result:
	case <-time.After(5 * time.Second):
		t.Fatal("timeout waiting for ownership_make_point")
	}

	obj, ok := v.(*Object)
	if !ok {
		t.Fatalf("expected *Object, got %T: %v", v, v)
	}
	if x, err := obj.Get("x"); err != nil || fmt.Sprint(x) != "4" {
		t.Fatalf("expected x = 4, got %v (err: %v)", x, err)
	}

	obj, v = nil, nil
	runFinalizers()
}

// Await a future and block until it is resolved or rejected (Future.Await does not wait for the result)
func awaitFuture(t *testing.T, fut *Future) (interface{}, error) {
	type futureResult struct {
		value interface{}
		err   error
	}

	result := make(chan futureResult, 1)

	val, err := fut.Await(
		func(v, _ interface{}) interface{} {
			result <- futureResult{v, nil}
			return nil
		},
		func(v, _ interface{}) interface{} {
			if e, ok := v.(error); ok {
				result <- futureResult{nil, e}
			} else {
				result <- futureResult{nil, fmt.Errorf("%v", v)}
			}
			return nil
		},
		nil)

	if err != nil {
		t.Fatalf("await of future failed: %v", err)
	}

	if _, ok := val.(*Future); !ok {
		t.Fatalf("expected *Future from await, got %T", val)
	}

	select {
	case r := <-result:
		return r.value, r.err
	case <-time.After(5 * time.Second):
		t.Fatal("timeout waiting for the future to be resolved or rejected")
	}

	return nil, nil
}

func TestNodeJSOwnershipFuture(t *testing.T) {
	requireLoader(t, "node")
	script := `
        module.exports = {
            ownershipNestedPromise: () => [Promise.resolve(5)],
            ownershipMakePromise: () => Promise.resolve(6),
            ownershipIdentity: (x) => x,
        };
        `
	if err := LoadFromMemory("node", script); err != nil {
		t.Fatalf("failed to load script: %v", err)
	}

	// Future inside of an array
	val, err := Call("ownershipNestedPromise")
	if err != nil {
		t.Fatalf("call to ownershipNestedPromise failed: %v", err)
	}
	array, ok := val.([]interface{})
	if !ok || len(array) != 1 {
		t.Fatalf("expected array of 1 element, got %T: %v", val, val)
	}
	fut, ok := array[0].(*Future)
	if !ok {
		t.Fatalf("expected *Future, got %T", array[0])
	}
	if res, err := awaitFuture(t, fut); err != nil || fmt.Sprint(res) != "5" {
		t.Fatalf("expected 5, got %v (err: %v)", res, err)
	}

	// Future passed as argument, the argument value is destroyed after the call
	val, err = Call("ownershipMakePromise")
	if err != nil {
		t.Fatalf("call to ownershipMakePromise failed: %v", err)
	}
	arg, ok := val.(*Future)
	if !ok {
		t.Fatalf("expected *Future, got %T", val)
	}
	// TODO: NodeJS loader does not support futures as arguments yet, the call fails but the argument
	// is copied and destroyed anyway, so the future must remain valid after it. Check the error and the
	// returned future once NodeJS loader implements it
	Call("ownershipIdentity", arg)
	if res, err := awaitFuture(t, arg); err != nil || fmt.Sprint(res) != "6" {
		t.Fatalf("expected 6, got %v (err: %v)", res, err)
	}

	array, fut, arg = nil, nil, nil
	runFinalizers()
}

func TestNodeJSFuture(t *testing.T) {
	requireLoader(t, "node")
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

	res, err := awaitFuture(t, fut)
	if err != nil {
		t.Fatalf("await returned unexpected error: %v", err)
	}
	if res != 50 && res != float64(50) {
		t.Fatalf("expected 50, got %v", res)
	}

	rejected := make(chan string, 1)

	failVal, err := Await("asyncFail",
		func(interface{}, interface{}) interface{} {
			log.Println("from go resolve")
			rejected <- ""
			return nil
		},
		func(v interface{}, _ interface{}) interface{} {
			log.Println("from go reject")
			rejected <- fmt.Sprint(v)
			return nil
		},
		nil,
		"database error")
	if err != nil {
		t.Fatalf("call to asyncFail failed: %v", err)
	}

	failFut, ok := failVal.(*Future)
	if !ok || failFut == nil {
		t.Fatalf("expected *Future, got %T", failVal)
	}

	select {
	case msg := <-rejected:
		if !strings.Contains(msg, "database error") {
			t.Fatalf("expected rejection with 'database error', got '%s'", msg)
		}
	case <-time.After(5 * time.Second):
		t.Fatal("timeout waiting for asyncFail to be rejected")
	}

	// The rejection has been handled by the reject callback, so the chained future resolves with its return value (nil)
	res, awaitErr := awaitFuture(t, failFut)
	if awaitErr != nil || res != nil {
		t.Fatalf("expected chained future to resolve with nil, got %v (err: %v)", res, awaitErr)
	}
}

func TestGoRoutineLeaks(t *testing.T) {
	requireLoader(t, "node")
	// This test is for race condition when shutdown occur with non-empty queue which triggers a deadlock and a goroutine leak
	// Start the port and fill the queue
	buffer := "module.exports = { leak_test: (x) => x }"
	if err := LoadFromMemory("node", buffer); err != nil {
		t.Fatal(err)
	}

	const ws = 100
	stop := make(chan interface{})
	shutdown := make(chan interface{})
	var wg sync.WaitGroup
	// Start concurrent callers
	for i := 0; i < ws; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			for {
				select {
				case <-stop:
					return
				default:
					Call("leak_test", "warmup")
				}
			}
		}()
	}

	// Call Destroy() to start shutdown concurrently with worker register in queue
	time.Sleep(2 * time.Millisecond)
	go func() {
		Destroy()
		close(stop)
		wg.Wait()
		close(shutdown)
	}()

	var timedOut bool
	select {
	case <-shutdown:
		timedOut = false
	case <-time.After(5 * time.Second):
		timedOut = true
	}
	// Getting the goroutine leaks and deadlocks profile and store it
	if err := writeProfile("goroutine", "goroutine_leaks.pprof"); err != nil {
		t.Fatal(err)
	}
	if timedOut {
		t.Error("Shutdown deadlock: goroutine leaks detected")
	}
}

func TestProfilesServer(t *testing.T) {
	mode := os.Getenv("MODE")
	if mode != "Debug" {
		return
	}
	// Use http instead of curl for cross-platform support
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

		// Create .pprof file for go tool pprof
		var filePath string
		if filePath = os.Getenv("PPROFDIR"); filePath == "" {
			filePath, _ = os.Getwd()
		}
		profFile, err := os.Create(filePath + "/" + req[0] + ".pprof")
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

// Await n calls to the benchmark function and wait until all of them are resolved, it can be called
// from RunParallel goroutines, so it must not use b.Fatal or b.Skip (they are only valid in the benchmark goroutine)
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
			b.Error(err)
			// Release the calls that have not been awaited
			wg.Add(i - n)
			break
		}
	}

	wg.Wait()
}

func BenchmarkNodeJSSequential(b *testing.B) {
	requireLoader(b, "node")
	for i := 0; i < b.N; i++ {
		benchmarkNodeJS(b, 5)
	}
}

func BenchmarkNodeJSParallel(b *testing.B) {
	requireLoader(b, "node")
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			benchmarkNodeJS(b, 5)
		}
	})
}

func TestMapConsumerTypeSwitch(t *testing.T) {
	// Scenario 1: Uniform string keys (Fast path)
	// Output: map[string]interface{}
	inputStr := map[string]interface{}{"key": 1}
	var ptrStr unsafe.Pointer
	goToValue(inputStr, &ptrStr)
	defer valueDestroy(ptrStr)

	vStr := valueToGo(ptrStr)
	switch m := vStr.(type) {
	case map[string]interface{}:
		if val, ok := m["key"]; !ok || val != 1 {
			t.Errorf("Consumer failed to read from map[string]interface{}: %v", m)
		}
	default:
		t.Fatalf("Expected map[string]interface{}, got %T", vStr)
	}

	// Scenario 2: Uniform non-string keys (Reflect path)
	// Output: map[int]interface{}
	inputInt := map[int]string{1: "one"}
	var ptrInt unsafe.Pointer
	goToValue(inputInt, &ptrInt)
	defer valueDestroy(ptrInt)

	vInt := valueToGo(ptrInt)
	switch m := vInt.(type) {
	case map[int]interface{}:
		if val, ok := m[1]; !ok || val != "one" {
			t.Errorf("Consumer failed to read from map[int]interface{}: %v", m)
		}
	default:
		t.Fatalf("Expected map[int]interface{}, got %T", vInt)
	}

	// Scenario 3: Mixed keys (Fallback path)
	// Output: map[interface{}]interface{}
	inputMixed := map[interface{}]interface{}{"str": 1, 2: "int"}
	var ptrMixed unsafe.Pointer
	goToValue(inputMixed, &ptrMixed)
	defer valueDestroy(ptrMixed)

	vMixed := valueToGo(ptrMixed)
	switch m := vMixed.(type) {
	case map[interface{}]interface{}:
		if val, ok := m["str"]; !ok || val != 1 {
			t.Errorf("Consumer failed to read from map[interface{}]interface{}: %v", m)
		}
		if val, ok := m[2]; !ok || val != "int" {
			t.Errorf("Consumer failed to read from map[interface{}]interface{}: %v", m)
		}
	default:
		t.Fatalf("Expected map[interface{}]interface{}, got %T", vMixed)
	}
}

// Values of the registry tests are counters of the times that they have been destroyed
func registryTestValues(n int) []unsafe.Pointer {
	values := make([]unsafe.Pointer, n)
	for i := range values {
		values[i] = unsafe.Pointer(new(int64))
	}
	return values
}

func registryTestDestroy(value unsafe.Pointer) {
	atomic.AddInt64((*int64)(value), 1)
}

// Check that each value has been destroyed exactly once
func registryTestCheck(t *testing.T, values []unsafe.Pointer) {
	t.Helper()
	for i, value := range values {
		if count := atomic.LoadInt64((*int64)(value)); count != 1 {
			t.Fatalf("value %d destroyed %d times, expected once", i, count)
		}
	}
}

func TestRegistrySaveDelete(t *testing.T) {
	r := new(valueRegistry)
	value := registryTestValues(1)[0]

	if handle := r.save(nil); handle != 0 {
		t.Fatalf("nil value registered with handle %d", handle)
	}

	handle := r.save(value)

	if handle == 0 {
		t.Fatal("value registered with invalid handle 0")
	}

	if v := r.delete(0); v != nil {
		t.Fatal("invalid handle 0 released a value")
	}

	if v := r.delete(handle); v != value {
		t.Fatalf("delete returned %p, expected %p", v, value)
	}

	if v := r.delete(handle); v != nil {
		t.Fatal("value released twice")
	}
}

func TestRegistryReuse(t *testing.T) {
	r := new(valueRegistry)
	values := registryTestValues(2)

	old := r.save(values[0])
	r.delete(old)
	handle := r.save(values[1])

	if uint32(old) != uint32(handle) {
		t.Fatalf("slot %d not reused, new slot %d", uint32(old), uint32(handle))
	}

	if old == handle {
		t.Fatal("reused slot has the same tag")
	}

	// A finalizer of the old wrapper must not release the value of the new one
	if v := r.delete(old); v != nil {
		t.Fatal("old handle released the value of the reused slot")
	}

	if v := r.delete(handle); v != values[1] {
		t.Fatalf("delete returned %p, expected %p", v, values[1])
	}

	// The free list must be used instead of new slots
	for i := 0; i < 1000; i++ {
		r.delete(r.save(values[0]))
	}

	if r.top != 1 {
		t.Fatalf("free list not reused, %d slots used", r.top)
	}
}

func TestRegistryDrain(t *testing.T) {
	r := new(valueRegistry)

	// Use more than one chunk
	values := registryTestValues(registryChunkSize*2 + 1)
	handles := make([]uint64, len(values))

	for i, value := range values {
		handles[i] = r.save(value)
	}

	// Release half of the values as the finalizers do
	for i := 0; i < len(handles); i += 2 {
		registryTestDestroy(r.delete(handles[i]))
	}

	r.drain(registryTestDestroy)
	registryTestCheck(t, values)

	// The finalizers that run after the drain must not release anything
	for _, handle := range handles {
		if v := r.delete(handle); v != nil {
			t.Fatal("value released after the drain")
		}
	}

	// The slots must be reused after the drain
	top := r.top

	for _, value := range values {
		r.save(value)
	}

	if r.top != top {
		t.Fatalf("slots not reused after the drain, %d slots used, expected %d", r.top, top)
	}
}

func TestRegistryConcurrent(t *testing.T) {
	const (
		goroutines = 8
		iterations = 1000
		live       = 16 // Values kept alive by each goroutine
	)

	r := new(valueRegistry)
	values := registryTestValues(goroutines * iterations)
	remaining := make([][]uint64, goroutines)

	var wg sync.WaitGroup

	// Save and delete from many goroutines, as the wrappers and their finalizers do
	for g := 0; g < goroutines; g++ {
		wg.Add(1)
		go func(g int) {
			defer wg.Done()
			var handles []uint64
			for i := 0; i < iterations; i++ {
				handles = append(handles, r.save(values[g*iterations+i]))
				if len(handles) > live {
					registryTestDestroy(r.delete(handles[0]))
					handles = handles[1:]
				}
			}
			remaining[g] = handles
		}(g)
	}

	wg.Wait()

	// Each goroutine has at most live + 1 slots in use and one slot being pushed to the free list
	if max := uint32(goroutines * (live + 2)); r.top > max {
		t.Fatalf("free list not reused, %d slots used, expected at most %d", r.top, max)
	}

	// Delete the remaining values while draining, as the finalizers that run during the destruction of MetaCall
	start := make(chan struct{})

	for g := 0; g < goroutines; g++ {
		wg.Add(1)
		go func(handles []uint64) {
			defer wg.Done()
			<-start
			for _, handle := range handles {
				if v := r.delete(handle); v != nil {
					registryTestDestroy(v)
				}
			}
		}(remaining[g])
	}

	close(start)
	r.drain(registryTestDestroy)
	wg.Wait()

	registryTestCheck(t, values)
}

func BenchmarkRegistrySequential(b *testing.B) {
	r := new(valueRegistry)
	value := registryTestValues(1)[0]
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		r.delete(r.save(value))
	}
}

func BenchmarkRegistryParallel(b *testing.B) {
	r := new(valueRegistry)
	value := registryTestValues(1)[0]
	b.ResetTimer()
	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			r.delete(r.save(value))
		}
	})
}
