# Лабораторная работа 2 Вариант 5

Все задания реализованы на **C++17 и Go**.

- C++: [laba2_variant5/cpp](laba2_variant5/cpp) — собственные хеш-таблица, Set и LRU, структуры личной ЛР1, решения и тесты.
- Go: `laba2_variant5/main.go`, `str/`, `tasks/`.
- Результаты C++: [cpp/reports](laba2_variant5/cpp/reports) — 13 групп, 80382 проверки; 583 сценария и 1125 CLI-запусков; эксперимент хеширования.

```sh
cd laba2_variant5
make cpp
./laba2-cpp --task 3 < examples/task3.txt
make test-cpp
```

Полная проверка двух языков:

```sh
make test check
make experiment-cpp
```

Для C++ нужны компилятор C++17 и Make; для общей проверки — также Go и Python 3.
Сборка C++ проверена на macOS arm64. Файловый интерфейс использует POSIX и рассчитан на macOS/Linux.

Варианты: 1 — порядок задач; 2 — файловое множество; 3 — геном; 4 — ударения; 5 — восстановление BST; 6.1 — линейное и двойное хеширование; 6.2 — подчинённые; 7 — LRU.

Инструкции и ограничения: [C++ README](laba2_variant5/cpp/README.md).
