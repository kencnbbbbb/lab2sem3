package tasks

import (
	"bytes"
	"fmt"
	"laba2/str"
	"math/rand"
	"os"
	"path/filepath"
	"reflect"
	"strings"
	"testing"
)

func TestTaskOrder(t *testing.T) {
	cases := []struct {
		names    []string
		edges    [][2]string
		possible bool
	}{
		{[]string{"A", "B", "C"}, [][2]string{{"A", "B"}, {"B", "C"}}, true},
		{[]string{"A", "B", "C", "D"}, [][2]string{{"A", "B"}, {"A", "D"}, {"B", "C"}, {"C", "A"}}, false},
		{[]string{"A"}, [][2]string{{"A", "A"}}, false},
		{[]string{"A", "B", "C", "D"}, [][2]string{{"A", "B"}, {"A", "C"}, {"B", "D"}, {"C", "D"}, {"A", "B"}}, true},
		{nil, nil, true},
	}
	for _, c := range cases {
		order, e := TaskOrder(c.names, c.edges)
		if (e == nil) != c.possible {
			t.Fatal(c, e)
		}
		if e == nil {
			pos := map[string]int{}
			for i, n := range order {
				pos[n] = i
			}
			if len(pos) != len(c.names) {
				t.Fatal("missing task")
			}
			for _, edge := range c.edges {
				if pos[edge[1]] >= pos[edge[0]] {
					t.Fatal("dependency order")
				}
			}
		}
	}
	if _, e := TaskOrder([]string{"A"}, [][2]string{{"A", "B"}}); e == nil {
		t.Fatal("unknown task")
	}
}
func TestGenome(t *testing.T) {
	for _, c := range []struct {
		a, b string
		want int
	}{{"ABBACAB", "BCABB", 4}, {"AAAA", "AA", 3}, {"A", "A", 0}, {"ABC", "XYZ", 0}, {"ABAB", "AB", 2}} {
		if got := GenomeSimilarity(c.a, c.b); got != c.want {
			t.Fatal(c, got)
		}
	}
	if got := GenomeSimilarity(strings.Repeat("A", 100000), "AA"); got != 99999 {
		t.Fatal(got)
	}
}
func TestStress(t *testing.T) {
	d := []string{"cAnnot", "cannOt", "fOund", "pAge"}
	for _, c := range []struct {
		text string
		want int
	}{{"thE pAge cAnnot be fouNd", 2}, {"The PAGE cannot be found", 4}, {"cAnnot cannOt", 0}, {"x X XX", 2}} {
		if got := StressErrors(d, strings.Fields(c.text)); got != c.want {
			t.Fatal(c, got)
		}
	}
	if StressErrors(nil, []string{"тЕст", "ТЕст", "тест"}) != 2 {
		t.Fatal("unicode")
	}
}
func TestRecoverTreeAllSwaps(t *testing.T) {
	for _, input := range []string{"4 2 1 # # 3 # # 6 5 # # 7 # #", "1 # 2 # 3 # 4 # #", "2 1 # # 3 # #"} {
		for a := 0; a < strings.Count(input, "#")-2; a++ {
			for b := a + 1; b < strings.Count(input, "#")-1; b++ {
				tree, e := ParseTree(strings.Fields(input))
				if e != nil {
					t.Fatal(e)
				}
				nodes := []*str.TreeNode{}
				var visit func(*str.TreeNode)
				visit = func(n *str.TreeNode) {
					if n != nil {
						visit(n.Left)
						nodes = append(nodes, n)
						visit(n.Right)
					}
				}
				visit(tree.Root)
				if b >= len(nodes) {
					continue
				}
				links := make([][2]*str.TreeNode, len(nodes))
				for i, n := range nodes {
					links[i] = [2]*str.TreeNode{n.Left, n.Right}
				}
				nodes[a].Data, nodes[b].Data = nodes[b].Data, nodes[a].Data
				if !RecoverTree(tree) || !ValidTree(tree) {
					t.Fatal("not recovered", input, a, b)
				}
				if Preorder(tree) != input {
					t.Fatal("wrong values")
				}
				for i, n := range nodes {
					if links[i] != [2]*str.TreeNode{n.Left, n.Right} {
						t.Fatal("shape changed")
					}
				}
			}
		}
	}
	for _, input := range []string{"1 #", "1 # # 2", "x # #"} {
		if _, e := ParseTree(strings.Fields(input)); e == nil {
			t.Fatal("malformed tree")
		}
	}
	for _, input := range []string{"#", "1 # #", "2 2 # # 2 # #", "1 3 # # 2 # #"} {
		tree, e := ParseTree(strings.Fields(input))
		if e != nil {
			t.Fatal(e)
		}
		before := Preorder(tree)
		if RecoverTree(tree) {
			t.Fatal("accepted impossible tree")
		}
		if Preorder(tree) != before {
			t.Fatal("failure mutated tree")
		}
	}
}
func TestEmployees(t *testing.T) {
	pairs := [][2]string{{"A", "B"}, {"C", "B"}, {"D", "E"}, {"B", "E"}, {"E", "E"}}
	for _, mode := range []str.ProbeMode{str.Linear, str.Double} {
		got, e := Employees(pairs, mode)
		if e != nil {
			t.Fatal(e)
		}
		want := []EmployeeResult{{"A", nil}, {"B", []string{"A", "C"}}, {"C", nil}, {"D", nil}, {"E", []string{"A", "B", "C", "D"}}}
		if !reflect.DeepEqual(got, want) {
			t.Fatal(got)
		}
	}
	for _, pairs := range [][][2]string{{{"A", "B"}, {"B", "A"}}, {{"A", "B"}, {"A", "C"}}} {
		if _, e := Employees(pairs, str.Double); e == nil {
			t.Fatal("invalid hierarchy")
		}
	}
	got, e := Employees([][2]string{{"A", "B"}, {"A", "B"}, {"C", "D"}}, str.Double)
	if e != nil || len(got) != 4 {
		t.Fatal("duplicates or forest")
	}
}
func TestLRUExamples(t *testing.T) {
	for _, c := range []struct{ input, want string }{{"2 2 SET 1 2 GET 1", "2\n"}, {"2 8 SET 1 2 SET 2 3 SET 1 5 SET 4 5 SET 6 7 GET 4 SET 1 2 GET 3", "5 -1\n"}, {"0 3 SET 1 5 GET 1 GET 2", "-1 -1\n"}, {"2 3 SET 01 7 GET 1 GET 01", "7 7\n"}} {
		var out bytes.Buffer
		if e := RunTask7(strings.NewReader(c.input), &out); e != nil || out.String() != c.want {
			t.Fatal(c, out.String(), e)
		}
	}
}
func TestSetFile(t *testing.T) {
	path := filepath.Join(t.TempDir(), "sets.json")
	var out bytes.Buffer
	for _, q := range []string{"SETADD a 1 2 1", "SETADD b 9", "SETDEL a 1", "SET_AT a 1", "SET_AT a 2", "SET_AT b 9"} {
		if e := Parsing(path, q, &out); e != nil {
			t.Fatal(e)
		}
	}
	if out.String() != "false\ntrue\ntrue\n" {
		t.Fatal(out.String())
	}
	before, _ := os.ReadFile(path)
	if e := Parsing(path, "BAD a", &out); e == nil {
		t.Fatal("bad query")
	}
	after, _ := os.ReadFile(path)
	if !bytes.Equal(before, after) {
		t.Fatal("file changed")
	}
	os.WriteFile(path, []byte("broken"), 0600)
	if e := Parsing(path, "SETADD a 1", &out); e == nil {
		t.Fatal("corrupt file overwritten")
	}
	after, _ = os.ReadFile(path)
	if string(after) != "broken" {
		t.Fatal("corrupt file changed")
	}
}
func TestLargeDAG(t *testing.T) {
	const n = 10000
	names := make([]string, n)
	edges := make([][2]string, n-1)
	for i := range names {
		names[i] = fmt.Sprint(i)
		if i > 0 {
			edges[i-1] = [2]string{names[i], names[i-1]}
		}
	}
	order, e := TaskOrder(names, edges)
	if e != nil || len(order) != n || order[0] != "0" {
		t.Fatal(e)
	}
}

func TestRandomGraphs(t *testing.T) {
	// Независимый эталон: алгоритм Кана (очередь вместо DFS).
	rng := rand.New(rand.NewSource(51))
	for sample := 0; sample < 300; sample++ {
		n := 1 + rng.Intn(20)
		names := make([]string, n)
		for i := range names {
			names[i] = fmt.Sprint(i)
		}
		edges := [][2]string{}
		out := make([][]int, n)
		degree := make([]int, n)
		for a := 0; a < n; a++ {
			for b := 0; b < n; b++ {
				if rng.Intn(15) == 0 {
					edges = append(edges, [2]string{names[a], names[b]})
					out[b] = append(out[b], a)
					degree[a]++
				}
			}
		}
		queue := []int{}
		for i, d := range degree {
			if d == 0 {
				queue = append(queue, i)
			}
		}
		for head := 0; head < len(queue); head++ {
			for _, v := range out[queue[head]] {
				degree[v]--
				if degree[v] == 0 {
					queue = append(queue, v)
				}
			}
		}
		_, e := TaskOrder(names, edges)
		if (e == nil) != (len(queue) == n) {
			t.Fatal("DFS disagrees with Kahn", sample)
		}
	}
}
