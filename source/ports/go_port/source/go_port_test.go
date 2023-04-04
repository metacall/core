package metacall

import (
	"log"
	"os"
	"reflect"
	"sync"
	"testing"
	"unsafe"
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

func TestValues(t *testing.T) {
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
		{"int_min", int(-2147483648), int(-2147483648)},
		{"int_max", int(2147483647), int(2147483647)},
		{"long", int64(3), int64(3)},
		{"long_min", int64(-9223372036854775808), int64(-9223372036854775808)},
		{"long_max", int64(9223372036854775807), int64(9223372036854775807)},
		{"float", float32(1.0), float32(1.0)},
		{"float_min", float32(1.2e-38), float32(1.2e-38)},
		{"float_max", float32(3.4e+38), float32(3.4e+38)},
		{"double", float64(1.0), float64(1.0)},
		{"double_min", float64(2.3e-308), float64(2.3e-308)},
		{"double_max", float64(1.7e+308), float64(1.7e+308)},
		{"string", "hello", "hello"},
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
