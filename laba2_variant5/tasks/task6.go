package tasks

import (
	"fmt"
	"io"
	"laba2/str"
	"sort"
	"strconv"
)

type EmployeeResult struct {
	Name         string
	Subordinates []string
}

func Employees(pairs [][2]string, mode str.ProbeMode) ([]EmployeeResult, error) {
	table := str.NewHashTable[int](len(pairs)*2, mode)
	names := []string{}
	children := [][]int{}
	parents := []int{}
	id := func(name string) int {
		if i, ok := table.Get(name); ok {
			return i
		}
		i := len(names)
		table.Put(name, i)
		names = append(names, name)
		children = append(children, nil)
		parents = append(parents, -1)
		return i
	}
	for _, p := range pairs {
		a, b := id(p[0]), id(p[1])
		if parents[a] != -1 {
			if parents[a] == b {
				continue
			}
			return nil, fmt.Errorf("у %s несколько менеджеров", p[0])
		}
		parents[a] = b
		if a != b {
			children[b] = append(children[b], a)
		}
	}
	// Сначала строим постпорядок без рекурсии, затем объединяем списки подчинённых.
	state := make([]uint8, len(names))
	post := []int{}
	stack := str.NewStack()
	for start := range names {
		if state[start] != 0 {
			continue
		}
		stack.PushBack(strconv.Itoa(start))
		for stack.List.Tail != nil {
			v, _ := strconv.Atoi(stack.Pop())
			if v < 0 {
				v = -v - 1
				state[v] = 2
				post = append(post, v)
				continue
			}
			if state[v] == 2 {
				continue
			}
			if state[v] == 1 {
				return nil, fmt.Errorf("цикл в иерархии сотрудников")
			}
			state[v] = 1
			stack.PushBack(strconv.Itoa(-v - 1))
			for _, child := range children[v] {
				stack.PushBack(strconv.Itoa(child))
			}
		}
	}
	all := make([][]string, len(names))
	for _, v := range post {
		for _, c := range children[v] {
			all[v] = append(all[v], names[c])
			all[v] = append(all[v], all[c]...)
		}
		sort.Strings(all[v])
	}
	results := make([]EmployeeResult, len(names))
	for i, name := range names {
		results[i] = EmployeeResult{name, all[i]}
	}
	sort.Slice(results, func(i, j int) bool { return results[i].Name < results[j].Name })
	return results, nil
}
func RunTask6(r io.Reader, w io.Writer, mode str.ProbeMode) error {
	var n int
	if _, e := fmt.Fscan(r, &n); e != nil {
		return e
	}
	if n < 0 {
		return fmt.Errorf("отрицательное число пар")
	}
	pairs := make([][2]string, n)
	for i := range pairs {
		if _, e := fmt.Fscan(r, &pairs[i][0], &pairs[i][1]); e != nil {
			return e
		}
	}
	result, e := Employees(pairs, mode)
	if e != nil {
		return e
	}
	for _, row := range result {
		fmt.Fprintf(w, "%s-%d", row.Name, len(row.Subordinates))
		if len(row.Subordinates) > 0 {
			fmt.Fprint(w, ":")
			for _, name := range row.Subordinates {
				fmt.Fprint(w, " ", name)
			}
		}
		fmt.Fprintln(w)
	}
	return nil
}
