package tasks

import (
	"fmt"
	"io"
	"laba2/str"
	"strconv"
)

// Зависимость A B означает: A можно выполнить только после B.
func TaskOrder(names []string, edges [][2]string) ([]string, error) {
	ids := str.NewHashTable[int](len(names)*2, str.Double)
	for i, n := range names {
		if _, ok := ids.Get(n); ok {
			return nil, fmt.Errorf("повтор задачи %s", n)
		}
		ids.Put(n, i)
	}
	adj := make([][]int, len(names))
	for _, e := range edges {
		a, ok := ids.Get(e[0])
		b, ok2 := ids.Get(e[1])
		if !ok || !ok2 {
			return nil, fmt.Errorf("неизвестная задача в зависимости %v", e)
		}
		adj[a] = append(adj[a], b)
	}
	color := make([]uint8, len(names))
	order := make([]string, 0, len(names))
	stack := str.NewStack()
	for start := range names {
		if color[start] != 0 {
			continue
		}
		stack.PushBack(strconv.Itoa(start))
		for stack.List.Tail != nil {
			code, _ := strconv.Atoi(stack.Pop())
			if code < 0 {
				v := -code - 1
				color[v] = 2
				order = append(order, names[v])
				continue
			}
			v := code
			if color[v] == 2 {
				continue
			}
			if color[v] == 1 {
				return nil, fmt.Errorf("цикл зависимостей")
			}
			color[v] = 1
			stack.PushBack(strconv.Itoa(-v - 1))
			for i := len(adj[v]) - 1; i >= 0; i-- {
				stack.PushBack(strconv.Itoa(adj[v][i]))
			}
		}
	}
	return order, nil
}
func RunTask1(r io.Reader, w io.Writer) error {
	var n, m int
	if _, e := fmt.Fscan(r, &n, &m); e != nil {
		return e
	}
	if n < 0 || m < 0 {
		return fmt.Errorf("отрицательный размер")
	}
	names := make([]string, n)
	for i := range names {
		if _, e := fmt.Fscan(r, &names[i]); e != nil {
			return e
		}
	}
	edges := make([][2]string, m)
	for i := range edges {
		if _, e := fmt.Fscan(r, &edges[i][0], &edges[i][1]); e != nil {
			return e
		}
	}
	// Ошибки ввода отличаем от отсутствия допустимого порядка.
	ids := str.NewSet(n * 2)
	for _, name := range names {
		if ids.Has(name) {
			return fmt.Errorf("повтор задачи")
		}
		ids.Add(name)
	}
	for _, e := range edges {
		if !ids.Has(e[0]) || !ids.Has(e[1]) {
			return fmt.Errorf("неизвестная задача")
		}
	}
	order, e := TaskOrder(names, edges)
	if e != nil {
		fmt.Fprintln(w, "Невозможно")
		return nil
	}
	fmt.Fprintln(w, "Возможно")
	for i, name := range order {
		if i > 0 {
			fmt.Fprint(w, " ")
		}
		fmt.Fprint(w, name)
	}
	fmt.Fprintln(w)
	return nil
}
