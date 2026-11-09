package tasks

import (
	"fmt"
	"io"
	"laba2/str"
	"strconv"
)

func RunTask7(r io.Reader, w io.Writer) error {
	var capacity, q int
	if _, e := fmt.Fscan(r, &capacity, &q); e != nil {
		return e
	}
	if capacity < 0 || q < 0 {
		return fmt.Errorf("отрицательный размер")
	}
	cache := str.NewLru(capacity)
	out := []string{}
	for i := 0; i < q; i++ {
		var op, key, value string
		if _, e := fmt.Fscan(r, &op, &key); e != nil {
			return e
		}
		if _, e := strconv.ParseInt(key, 10, 64); e != nil {
			return fmt.Errorf("ключ должен быть целым числом")
		}
		// Нормализация: 01 и 1 - один целочисленный ключ.
		k, _ := strconv.ParseInt(key, 10, 64)
		key = strconv.FormatInt(k, 10)
		switch op {
		case "GET":
			out = append(out, cache.Get(key))
		case "SET":
			if _, e := fmt.Fscan(r, &value); e != nil {
				return e
			}
			v, e := strconv.ParseInt(value, 10, 64)
			if e != nil {
				return fmt.Errorf("значение должно быть целым числом")
			}
			cache.Set(key, strconv.FormatInt(v, 10))
		default:
			return fmt.Errorf("неизвестная операция %s", op)
		}
	}
	for i, v := range out {
		if i > 0 {
			fmt.Fprint(w, " ")
		}
		fmt.Fprint(w, v)
	}
	fmt.Fprintln(w)
	return nil
}
