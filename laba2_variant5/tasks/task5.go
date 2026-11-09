package tasks

import (
	"fmt"
	"io"
	"laba2/str"
	"strconv"
	"strings"
)

// Корректируем только значения. Ссылки Left/Right остаются прежними.
func RecoverTree(t *str.Tree) bool {
	var first, second, prev *str.TreeNode
	stack := []*str.TreeNode{}
	cur := t.Root
	for cur != nil || len(stack) > 0 {
		for cur != nil {
			stack = append(stack, cur)
			cur = cur.Left
		}
		cur = stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		if prev != nil && prev.Data >= cur.Data {
			if first == nil {
				first = prev
			}
			second = cur
		}
		prev = cur
		cur = cur.Right
	}
	if first == nil || second == nil {
		return false
	}
	first.Data, second.Data = second.Data, first.Data
	if !ValidTree(t) {
		first.Data, second.Data = second.Data, first.Data
		return false
	}
	return true
}
func Inorder(t *str.Tree) []int {
	result := []int{}
	stack := []*str.TreeNode{}
	cur := t.Root
	for cur != nil || len(stack) > 0 {
		for cur != nil {
			stack = append(stack, cur)
			cur = cur.Left
		}
		cur = stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		result = append(result, cur.Data)
		cur = cur.Right
	}
	return result
}
func ValidTree(t *str.Tree) bool {
	a := Inorder(t)
	for i := 1; i < len(a); i++ {
		if a[i-1] >= a[i] {
			return false
		}
	}
	return true
}

// Префиксная запись с # сохраняет исходную структуру повреждённого дерева.
func ParseTree(tokens []string) (*str.Tree, error) {
	t := str.Tree{}
	slots := []**str.TreeNode{&t.Root}
	for _, s := range tokens {
		if len(slots) == 0 {
			return nil, fmt.Errorf("лишние узлы после конца дерева")
		}
		slot := slots[len(slots)-1]
		slots = slots[:len(slots)-1]
		if s == "#" {
			continue
		}
		v, e := strconv.Atoi(s)
		if e != nil {
			return nil, fmt.Errorf("неверный узел %q", s)
		}
		node := &str.TreeNode{Data: v}
		*slot = node
		slots = append(slots, &node.Right, &node.Left)
	}
	if len(slots) != 0 {
		return nil, fmt.Errorf("неполная запись дерева: используйте # для пустых детей")
	}
	return &t, nil
}
func Preorder(t *str.Tree) string {
	result := []string{}
	stack := []*str.TreeNode{t.Root}
	for len(stack) > 0 {
		n := stack[len(stack)-1]
		stack = stack[:len(stack)-1]
		if n == nil {
			result = append(result, "#")
			continue
		}
		result = append(result, strconv.Itoa(n.Data))
		stack = append(stack, n.Right, n.Left)
	}
	return strings.Join(result, " ")
}
func RunTask5(r io.Reader, w io.Writer) error {
	data, e := io.ReadAll(r)
	if e != nil {
		return e
	}
	t, e := ParseTree(strings.Fields(string(data)))
	if e != nil {
		return e
	}
	if !RecoverTree(t) {
		return fmt.Errorf("дерево не исправляется обменом ровно двух различных значений")
	}
	fmt.Fprintln(w, "Исправленное дерево (preorder):", Preorder(t))
	fmt.Fprint(w, "Inorder:")
	for _, v := range Inorder(t) {
		fmt.Fprint(w, " ", v)
	}
	fmt.Fprintln(w)
	return nil
}
