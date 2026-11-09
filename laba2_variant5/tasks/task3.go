package tasks

import (
	"fmt"
	"io"
	"laba2/str"
)

func GenomeSimilarity(a, b string) int {
	pairs := str.NewSet(1024)
	for i := 0; i+1 < len(b); i++ {
		pairs.Add(b[i : i+2])
	}
	count := 0
	for i := 0; i+1 < len(a); i++ {
		if pairs.Has(a[i : i+2]) {
			count++
		}
	}
	return count
}
func RunTask3(r io.Reader, w io.Writer) error {
	var a, b string
	if _, e := fmt.Fscan(r, &a, &b); e != nil {
		return e
	}
	for _, s := range []string{a, b} {
		if len(s) == 0 || len(s) > 100000 {
			return fmt.Errorf("длина генома: 1..100000")
		}
		for _, c := range s {
			if c < 'A' || c > 'Z' {
				return fmt.Errorf("геном должен содержать A..Z")
			}
		}
	}
	fmt.Fprintln(w, GenomeSimilarity(a, b))
	return nil
}
