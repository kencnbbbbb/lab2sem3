package main

import (
	"bufio"
	"flag"
	"fmt"
	"io"
	"laba2/str"
	"laba2/tasks"
	"os"
	"strings"
)

func run(task string, r io.Reader, w io.Writer, mode str.ProbeMode) error {
	switch task {
	case "1":
		return tasks.RunTask1(r, w)
	case "3":
		return tasks.RunTask3(r, w)
	case "4":
		return tasks.RunTask4(r, w)
	case "5":
		return tasks.RunTask5(r, w)
	case "6", "6.2":
		return tasks.RunTask6(r, w, mode)
	case "6.1":
		tasks.HashExperiment(w)
		return nil
	case "7":
		return tasks.RunTask7(r, w)
	}
	return fmt.Errorf("неизвестное задание %q", task)
}
func interactive() error {
	r := bufio.NewReader(os.Stdin)
	for {
		fmt.Println("Задания: 1, 2, 3, 4, 5, 6.1, 6.2, 7; 0 - выход.")
		line, e := r.ReadString('\n')
		if e == io.EOF && line == "" {
			return nil
		}
		if e != nil && e != io.EOF {
			return e
		}
		task := strings.TrimSpace(line)
		if task == "0" {
			return nil
		}
		if task == "2" {
			fmt.Println("Путь к файлу:")
			path, e := r.ReadString('\n')
			if e != nil {
				return e
			}
			fmt.Println("Запрос (например SETADD myset 42):")
			query, e := r.ReadString('\n')
			if e != nil {
				return e
			}
			if e = tasks.Parsing(strings.TrimSpace(path), strings.TrimSpace(query), os.Stdout); e != nil {
				fmt.Fprintln(os.Stderr, "Ошибка:", e)
			}
			continue
		}
		if task == "6.1" {
			tasks.HashExperiment(os.Stdout)
			continue
		}
		fmt.Println("Введите данные задания. Завершите отдельной строкой END:")
		var data strings.Builder
		for {
			line, e = r.ReadString('\n')
			if strings.TrimSpace(line) == "END" {
				break
			}
			data.WriteString(line)
			if e == io.EOF {
				break
			}
			if e != nil {
				return e
			}
		}
		if e = run(task, strings.NewReader(data.String()), os.Stdout, str.Double); e != nil {
			fmt.Fprintln(os.Stderr, "Ошибка:", e)
		}
	}
}
func main() {
	task := flag.String("task", "", "номер задания: 1, 3, 4, 5, 6.1, 6.2, 7")
	file := flag.String("file", "", "файл данных множества")
	query := flag.String("query", "", "SETADD, SETDEL, SET_AT или PRINT")
	method := flag.String("method", "double", "хеширование: linear или double (задание 6.2)")
	flag.Parse()
	var e error
	if flag.NArg() != 0 {
		e = fmt.Errorf("лишние аргументы")
	} else if *method != "linear" && *method != "double" {
		e = fmt.Errorf("метод: linear или double")
	} else if *file != "" || *query != "" {
		if *file == "" || *query == "" || *task != "" {
			e = fmt.Errorf("для множества задайте --file и --query без --task")
		} else {
			e = tasks.Parsing(*file, *query, os.Stdout)
		}
	} else if *task != "" {
		mode := str.Double
		if *method == "linear" {
			mode = str.Linear
		}
		e = run(*task, bufio.NewReader(os.Stdin), os.Stdout, mode)
	} else {
		e = interactive()
	}
	if e != nil {
		fmt.Fprintln(os.Stderr, "Ошибка:", e)
		os.Exit(1)
	}
}
