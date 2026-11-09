package str

// Порядок LRU хранится в LinkedList из личной ЛР1: голова — недавно
// использованный ключ, хвост — кандидат на вытеснение.
type cacheEntry struct {
	node  *Node
	value string
}
type Lru struct {
	capacity int
	table    *HashTable[*cacheEntry]
	list     LinkedList
}

func NewLru(capacity int) *Lru {
	if capacity < 0 {
		capacity = 0
	}
	return &Lru{capacity: capacity, table: NewHashTable[*cacheEntry](16, Double), list: NewLinkedList()}
}

// Удаление известного узла не требует линейного поиска по значению.
func (l *Lru) detach(n *Node) {
	if n.Prev != nil {
		n.Prev.Next = n.Next
	} else {
		l.list.Head = n.Next
	}
	if n.Next != nil {
		n.Next.Prev = n.Prev
	} else {
		l.list.Tail = n.Prev
	}
	n.Prev = nil
	n.Next = nil
}
func (l *Lru) front(n *Node) {
	n.Next = l.list.Head
	if l.list.Head != nil {
		l.list.Head.Prev = n
	} else {
		l.list.Tail = n
	}
	l.list.Head = n
}
func (l *Lru) Get(key string) string {
	entry, ok := l.table.Get(key)
	if !ok {
		return "-1"
	}
	l.detach(entry.node)
	l.front(entry.node)
	return entry.value
}
func (l *Lru) Set(key, value string) {
	if l.capacity == 0 {
		return
	}
	if entry, ok := l.table.Get(key); ok {
		entry.value = value
		l.detach(entry.node)
		l.front(entry.node)
		return
	}
	if l.table.Len() == l.capacity {
		oldKey := l.list.PopBack()
		l.table.Delete(oldKey)
	}
	l.list.PushFront(key)
	l.table.Put(key, &cacheEntry{node: l.list.Head, value: value})
}
