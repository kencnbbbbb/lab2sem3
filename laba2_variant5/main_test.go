package main

import (
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

func TestCLI(t *testing.T) {
	binary := filepath.Join(t.TempDir(), "laba2")
	cmd := exec.Command("go", "build", "-o", binary, ".")
	if output, e := cmd.CombinedOutput(); e != nil {
		t.Fatalf("build: %s %v", output, e)
	}
	cases := []struct {
		args        []string
		input, want string
		fail        bool
	}{
		{[]string{"--task", "1"}, "3 2 A B C A B B C", "Возможно\nC B A\n", false},
		{[]string{"--task", "1"}, "3 3 A B C A B B C C A", "Невозможно\n", false},
		{[]string{"--task", "3"}, "ABBACAB\nBCABB\n", "4\n", false},
		{[]string{"--task", "4"}, "4\ncAnnot\ncannOt\nfOund\npAge\nthE pAge cAnnot be fouNd\n", "2\n", false},
		{[]string{"--task", "5"}, "3 1 # # 4 2 # # #", "Исправленное дерево (preorder): 2 1 # # 4 3 # # #\nInorder: 1 2 3 4\n", false},
		{[]string{"--task", "6.2", "--method", "linear"}, "5 A B C B D E B E E E", "A-0\nB-2: A C\nC-0\nD-0\nE-4: A B C D\n", false},
		{[]string{"--task", "6.2", "--method", "double"}, "5 A B C B D E B E E E", "A-0\nB-2: A C\nC-0\nD-0\nE-4: A B C D\n", false},
		{[]string{"--task", "7"}, "2 8 SET 1 2 SET 2 3 SET 1 5 SET 4 5 SET 6 7 GET 4 SET 1 2 GET 3", "5 -1\n", false},
		{[]string{"--task", "5"}, "1 #", "", true},
		{[]string{"--task", "1"}, "1 1 A A B", "", true},
		{[]string{"--task", "7"}, "2 1 SET 1 x", "", true},
		{[]string{"--task", "6.2"}, "2 A B B A", "", true},
		{[]string{"--task", "no"}, "", "", true},
		{[]string{"--method", "invalid"}, "", "", true},
	}
	for _, c := range cases {
		cmd := exec.Command(binary, c.args...)
		cmd.Stdin = strings.NewReader(c.input)
		out, e := cmd.CombinedOutput()
		if (e != nil) != c.fail {
			t.Fatalf("%v: %s %v", c.args, out, e)
		}
		if !c.fail && string(out) != c.want {
			t.Fatalf("%v: got %q want %q", c.args, out, c.want)
		}
	}
	path := filepath.Join(t.TempDir(), "data.json")
	for _, c := range []struct{ query, want string }{{"SETADD s 1 2", ""}, {"SET_AT s 1", "true\n"}, {"SETDEL s 1", ""}, {"SET_AT s 1", "false\n"}, {"PRINT s", "2\n"}} {
		out, e := exec.Command(binary, "--file", path, "--query", c.query).CombinedOutput()
		if e != nil || string(out) != c.want {
			t.Fatalf("persistence %s: %s %v", c.query, out, e)
		}
	}
	cmd = exec.Command(binary)
	cmd.Stdin = strings.NewReader("3\nABBACAB\nBCABB\nEND\n7\n2 2 SET 1 2 GET 1\nEND\n0\n")
	out, e := cmd.CombinedOutput()
	if e != nil || !strings.Contains(string(out), "4\n") || !strings.Contains(string(out), "2\n") {
		t.Fatalf("menu %s %v", out, e)
	}
}
