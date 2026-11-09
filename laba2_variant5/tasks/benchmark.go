package tasks

import (
	"fmt"
	"io"
	"laba2/str"
	"math/rand"
	"time"
)

// Одинаковая последовательность ключей для обоих методов. Заполнение < 70%, без роста.
func HashExperiment(w io.Writer) {
	fmt.Fprintln(w, "dataset,load,method,keys,insert_probes,hit_probes,miss_probes,delete_probes,insert_ns,hit_ns,miss_ns,delete_ns")
	for _, dataset := range []string{"random", "same_bucket"} {
		capacity := 16384
		if dataset == "same_bucket" {
			capacity = 1024
		}
		for _, load := range []int{25, 50, 65} {
			n := capacity * load / 100
			keys := make([]string, n)
			rng := rand.New(rand.NewSource(5))
			for i := range keys {
				if dataset == "random" {
					keys[i] = fmt.Sprintf("k%d_%016x", i, rng.Uint64())
				}
			}
			// same_bucket: специально отбираем ключи с одним начальным индексом.
			if dataset == "same_bucket" {
				keys = collidingKeys(n, capacity)
			}
			for _, mode := range []str.ProbeMode{str.Linear, str.Double} {
				h := str.NewHashTable[int](capacity, mode)
				name := "linear"
				if mode == str.Double {
					name = "double"
				}
				h.Probes = 0
				t := time.Now()
				for i, k := range keys {
					h.Put(k, i)
				}
				insertNS := time.Since(t).Nanoseconds()
				insertP := h.Probes
				h.Probes = 0
				t = time.Now()
				for _, k := range keys {
					if _, ok := h.Get(k); !ok {
						panic("потерян ключ")
					}
				}
				hitNS := time.Since(t).Nanoseconds()
				hitP := h.Probes
				h.Probes = 0
				t = time.Now()
				for _, k := range keys {
					h.Get("missing_" + k)
				}
				missNS := time.Since(t).Nanoseconds()
				missP := h.Probes
				h.Probes = 0
				t = time.Now()
				for _, k := range keys {
					h.Delete(k)
				}
				deleteNS := time.Since(t).Nanoseconds()
				deleteP := h.Probes
				fmt.Fprintf(w, "%s,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", dataset, load, name, n, insertP, hitP, missP, deleteP, insertNS, hitNS, missNS, deleteNS)
			}
		}
	}
}
func collidingKeys(n, capacity int) []string {
	// Сконструированные ключи: одинаковый DJB2 mod capacity, разные вторичные хеши.
	keys := make([]string, 0, n)
	for i := 0; len(keys) < n; i++ {
		s := fmt.Sprintf("collision_%d", i)
		h := uint64(5381)
		for j := 0; j < len(s); j++ {
			h = h*33 + uint64(s[j])
		}
		if h%uint64(capacity) == 0 {
			keys = append(keys, s)
		}
	}
	return keys
}
