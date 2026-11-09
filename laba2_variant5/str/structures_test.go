package str

import (
	"fmt"
	"math/rand"
	"testing"
)

func TestHashAgainstMap(t *testing.T) {
	for _, mode := range []ProbeMode{Linear, Double} {
		h := NewHashTable[int](17, mode)
		want := map[string]int{}
		rng := rand.New(rand.NewSource(5))
		for step := 0; step < 30000; step++ {
			key := fmt.Sprintf("key%d", rng.Intn(2000))
			switch rng.Intn(3) {
			case 0:
				v := rng.Int()
				h.Put(key, v)
				want[key] = v
			case 1:
				_, exists := want[key]
				if h.Delete(key) != exists {
					t.Fatal("delete", key)
				}
				delete(want, key)
			case 2:
				v, ok := h.Get(key)
				wv, wok := want[key]
				if v != wv || ok != wok {
					t.Fatal("get", key)
				}
			}
			if h.Len() != len(want) {
				t.Fatal("size")
			}
		}
		for k, v := range want {
			got, ok := h.Get(k)
			if !ok || got != v {
				t.Fatal("lost key", k)
			}
		}
	}
}
func TestHashTombstonesAndEmptyKey(t *testing.T) {
	for _, mode := range []ProbeMode{Linear, Double} {
		h := NewHashTable[int](16, mode)
		for round := 0; round < 100; round++ {
			for i := 0; i < 100; i++ {
				h.Put(fmt.Sprint(i), i)
			}
			for i := 0; i < 100; i++ {
				if !h.Delete(fmt.Sprint(i)) {
					t.Fatal("delete")
				}
			}
		}
		h.Put("", 7)
		h.Put("", 8)
		v, ok := h.Get("")
		if !ok || v != 8 || h.Len() != 1 {
			t.Fatal("empty/update")
		}
	}
}
func TestLRUAgainstReference(t *testing.T) {
	for capacity := 0; capacity < 8; capacity++ {
		cache := NewLru(capacity)
		values := map[string]string{}
		recent := []string{}
		rng := rand.New(rand.NewSource(52))
		touch := func(key string) {
			for i, k := range recent {
				if k == key {
					recent = append(recent[:i], recent[i+1:]...)
					break
				}
			}
			recent = append(recent, key)
		}
		for step := 0; step < 5000; step++ {
			key := fmt.Sprint(rng.Intn(15))
			if rng.Intn(2) == 0 {
				want, ok := values[key]
				if !ok {
					want = "-1"
				}
				if got := cache.Get(key); got != want {
					t.Fatalf("capacity %d step %d got %s want %s", capacity, step, got, want)
				}
				if ok {
					touch(key)
				}
			} else {
				v := fmt.Sprint(step)
				cache.Set(key, v)
				if capacity == 0 {
					continue
				}
				if _, ok := values[key]; !ok && len(values) == capacity {
					delete(values, recent[0])
					recent = recent[1:]
				}
				values[key] = v
				touch(key)
			}
		}
	}
}
func TestSet(t *testing.T) {
	s := NewSet(1)
	s.Add("AB")
	s.Add("AB")
	if len(s.Keys()) != 1 || !s.Has("AB") || !s.Del("AB") || s.Has("AB") || s.Del("AB") {
		t.Fatal("set operations")
	}
}

func TestLRULab1ListIntegration(t *testing.T) {
	cache := NewLru(2)
	cache.Set("a", "1")
	cache.Set("b", "2")
	a := cache.list.Tail
	b := cache.list.Head
	if a == nil || b == nil || a == b || a.Data != "a" || b.Data != "b" || a.Prev != b || b.Next != a {
		t.Fatal("initial links")
	}
	if cache.Get("a") != "1" || cache.list.Head != a || cache.list.Tail != b || a.Next != b || b.Prev != a || a.Prev != nil || b.Next != nil {
		t.Fatal("move known Lab1 node")
	}
	cache.Set("c", "3")
	if cache.Get("b") != "-1" || cache.list.Tail != a {
		t.Fatal("eviction")
	}
	if entry, ok := cache.table.Get("a"); !ok || entry.node != a {
		t.Fatal("node pointer lost")
	}
	cache.Set("a", "4")
	if cache.list.Head != a || cache.Get("a") != "4" {
		t.Fatal("update replaced node")
	}
}
