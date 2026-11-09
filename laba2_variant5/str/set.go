package str

// Set использует ту же собственную хеш-таблицу, что и задание 6.
type Set struct{ table *HashTable[struct{}] }

func NewSet(capacity int) Set        { return Set{NewHashTable[struct{}](capacity, Double)} }
func (s *Set) Add(value string)      { s.table.Put(value, struct{}{}) }
func (s *Set) Del(value string) bool { return s.table.Delete(value) }
func (s *Set) Has(value string) bool { _, ok := s.table.Get(value); return ok }
func (s *Set) Keys() []string        { return s.table.Keys() }
