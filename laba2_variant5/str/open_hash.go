package str

// Открытая адресация: линейное пробирование либо двойное хеширование.
type ProbeMode int

const (
	Linear ProbeMode = iota
	Double
)

type entry[V any] struct {
	key   string
	value V
	state uint8
}
type HashTable[V any] struct {
	slots         []entry[V]
	size, deleted int
	Mode          ProbeMode
	Probes        uint64 // Число просмотренных ячеек, включая внутреннее перехеширование.
}

func NewHashTable[V any](capacity int, mode ProbeMode) *HashTable[V] {
	c := 16
	for c < capacity {
		c *= 2
	}
	return &HashTable[V]{slots: make([]entry[V], c), Mode: mode}
}
func hashKey(key string) uint64 {
	h := uint64(5381)
	for i := 0; i < len(key); i++ {
		h = h*33 + uint64(key[i])
	}
	return h
}
func secondHash(key string) uint64 {
	h := uint64(14695981039346656037)
	for i := 0; i < len(key); i++ {
		h ^= uint64(key[i])
		h *= 1099511628211
	}
	return h
}
func (h *HashTable[V]) locate(key string) (int, bool) {
	mask := uint64(len(h.slots) - 1)
	index := hashKey(key) & mask
	step := uint64(1)
	if h.Mode == Double {
		step = secondHash(key) | 1
	} // Нечётный шаг взаимно прост с размером 2^k.
	firstDeleted := -1
	for n := 0; n < len(h.slots); n++ {
		h.Probes++
		e := &h.slots[index]
		if e.state == 0 {
			if firstDeleted >= 0 {
				return firstDeleted, false
			}
			return int(index), false
		}
		if e.state == 1 && e.key == key {
			return int(index), true
		}
		if e.state == 2 && firstDeleted < 0 {
			firstDeleted = int(index)
		}
		index = (index + step) & mask
	}
	return firstDeleted, false
}
func (h *HashTable[V]) Get(key string) (V, bool) {
	i, ok := h.locate(key)
	if ok {
		return h.slots[i].value, true
	}
	var zero V
	return zero, false
}
func (h *HashTable[V]) Put(key string, value V) {
	i, ok := h.locate(key)
	if ok {
		h.slots[i].value = value
		return
	}
	if (h.size+h.deleted+1)*10 >= len(h.slots)*7 {
		capacity := len(h.slots)
		if (h.size+1)*10 >= capacity*7 {
			capacity *= 2
		}
		h.resize(capacity)
		i, _ = h.locate(key)
	}
	if i < 0 {
		h.resize(len(h.slots) * 2)
		i, _ = h.locate(key)
	}
	if h.slots[i].state == 2 {
		h.deleted--
	}
	h.slots[i] = entry[V]{key: key, value: value, state: 1}
	h.size++
}
func (h *HashTable[V]) Delete(key string) bool {
	i, ok := h.locate(key)
	if !ok {
		return false
	}
	var zero V
	h.slots[i] = entry[V]{state: 2, value: zero}
	h.size--
	h.deleted++
	return true
}
func (h *HashTable[V]) resize(capacity int) {
	old := h.slots
	h.slots = make([]entry[V], capacity)
	h.size = 0
	h.deleted = 0
	for _, e := range old {
		if e.state == 1 {
			h.Put(e.key, e.value)
		}
	}
}
func (h *HashTable[V]) Len() int      { return h.size }
func (h *HashTable[V]) Capacity() int { return len(h.slots) }
func (h *HashTable[V]) Keys() []string {
	keys := make([]string, 0, h.size)
	for _, e := range h.slots {
		if e.state == 1 {
			keys = append(keys, e.key)
		}
	}
	return keys
}
