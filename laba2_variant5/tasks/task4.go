package tasks

import (
	"fmt"
	"io"
	"laba2/str"
	"strings"
	"unicode"
)

func StressErrors(dictionary, text []string) int {
	// Массив словаря - собственная структура из исходного проекта.
	words := str.NewArray(len(dictionary))
	known := str.NewSet(len(dictionary) * 2)
	correct := str.NewSet(len(dictionary) * 2)
	for _, word := range dictionary {
		words.PushBack(word)
	}
	for i := 0; i < words.Length(); i++ {
		word, _ := words.Get(i)
		known.Add(strings.ToLower(word))
		correct.Add(word)
	}
	errors := 0
	for _, word := range text {
		upper := 0
		for _, c := range word {
			if unicode.IsUpper(c) {
				upper++
			}
		}
		if upper != 1 || (known.Has(strings.ToLower(word)) && !correct.Has(word)) {
			errors++
		}
	}
	return errors
}
func RunTask4(r io.Reader, w io.Writer) error {
	var n int
	if _, e := fmt.Fscan(r, &n); e != nil {
		return e
	}
	if n < 0 {
		return fmt.Errorf("отрицательный размер словаря")
	}
	dict := make([]string, n)
	for i := range dict {
		if _, e := fmt.Fscan(r, &dict[i]); e != nil {
			return e
		}
		count := 0
		for _, c := range dict[i] {
			if unicode.IsUpper(c) {
				count++
			}
		}
		if count != 1 {
			return fmt.Errorf("в словарном слове должно быть одно ударение")
		}
	}
	// В режиме --task вход заканчивается после текста; в меню текст задаётся отдельным блоком.
	rest, e := io.ReadAll(r)
	if e != nil {
		return e
	}
	fmt.Fprintln(w, StressErrors(dict, strings.Fields(string(rest))))
	return nil
}
