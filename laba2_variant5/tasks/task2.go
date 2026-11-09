package tasks

import (
	"encoding/json"
	"fmt"
	"io"
	"laba2/str"
	"os"
	"path/filepath"
	"sort"
	"strings"
)

// Формат файла: JSON-объект {"имя множества": ["элемент", ...]}.
func Parsing(filename, query string, w io.Writer) error {
	fields := strings.Fields(query)
	if len(fields) < 2 {
		return fmt.Errorf("формат: ОПЕРАЦИЯ имя [элементы]")
	}
	op, name := fields[0], fields[1]
	if op != "SETADD" && op != "SETDEL" && op != "SET_AT" && op != "PRINT" {
		return fmt.Errorf("неизвестная операция %s", op)
	}
	if (op == "SETADD" || op == "SETDEL") && len(fields) < 3 {
		return fmt.Errorf("нужен хотя бы один элемент")
	}
	if op == "SET_AT" && len(fields) != 3 {
		return fmt.Errorf("SET_AT требует один элемент")
	}
	if op == "PRINT" && len(fields) != 2 {
		return fmt.Errorf("PRINT требует только имя")
	}
	data := map[string][]string{}
	raw, e := os.ReadFile(filename)
	if e != nil && !os.IsNotExist(e) {
		return e
	}
	if e == nil {
		if e = json.Unmarshal(raw, &data); e != nil {
			return fmt.Errorf("повреждён файл данных: %w", e)
		}
		if data == nil {
			return fmt.Errorf("ожидается JSON-объект")
		}
	}
	set := str.NewSet(len(data[name]) * 2)
	for _, v := range data[name] {
		set.Add(v)
	}
	switch op {
	case "SETADD":
		for _, v := range fields[2:] {
			set.Add(v)
		}
	case "SETDEL":
		for _, v := range fields[2:] {
			set.Del(v)
		}
	case "SET_AT":
		fmt.Fprintln(w, set.Has(fields[2]))
		return nil
	case "PRINT":
		keys := set.Keys()
		sort.Strings(keys)
		fmt.Fprintln(w, strings.Join(keys, " "))
		return nil
	}
	data[name] = set.Keys()
	sort.Strings(data[name])
	raw, e = json.MarshalIndent(data, "", "  ")
	if e != nil {
		return e
	}
	// Атомарная замена: исходный файл не портится при ошибке записи.
	tmp, e := os.CreateTemp(filepath.Dir(filename), ".laba2-*")
	if e != nil {
		return e
	}
	defer os.Remove(tmp.Name())
	if _, e = tmp.Write(append(raw, '\n')); e != nil {
		tmp.Close()
		return e
	}
	if e = tmp.Close(); e != nil {
		return e
	}
	return os.Rename(tmp.Name(), filename)
}
