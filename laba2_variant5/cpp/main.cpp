#include "tasks/tasks.h"
#include "tasks/support.h"
#include <iostream>
#include <sstream>
#include <map>

namespace {

int interactive() {
    while (true) {
        std::cout << "Задания: 1, 2, 3, 4, 5, 6.1, 6.2, 7; 0 - выход.\n";
        std::string task;
        if (!std::getline(std::cin, task) || task == "0") return 0;
        try {
            if (task == "2") {
                std::string path, query;
                std::cout << "Путь к файлу:\n";
                if (!std::getline(std::cin, path)) return 0;
                std::cout << "Запрос (например SETADD myset 42):\n";
                if (!std::getline(std::cin, query)) return 0;
                lab2::run_set(path, query, std::cout);
            } else if (task == "6.1") {
                lab2::hash_experiment(std::cout);
            } else {
                std::cout << "Введите данные задания. Завершите отдельной строкой END:\n";
                std::string data, line;
                while (std::getline(std::cin, line) && line != "END") {
                    data += line + '\n';
                }
                std::istringstream input(data);
                lab2::run_task(task, input, std::cout);
            }
        } catch (const std::exception& error) {
            std::cerr << "Ошибка: " << error.what() << '\n';
        }
    }
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::map<std::string, std::string> options;
        for (int i = 1; i < argc; ++i) {
            std::string key = argv[i];
            if (key == "--help" || key == "-h") {
                std::cout << "ЛР2 C++17. Без аргументов: меню.\n"
                             "--task 1|3|4|5|6.1|6.2|7 [--method linear|double]\n"
                             "--file sets.json --query \"SETADD имя элемент\"\n";
                return 0;
            }
            const auto equal = key.find('=');
            std::string value;
            if (equal != std::string::npos) {
                value = key.substr(equal + 1);
                key.resize(equal);
            } else {
                if (i + 1 >= argc) throw std::runtime_error("Не задано значение аргумента");
                value = argv[++i];
            }
            if (key.rfind("--", 0) == 0) key.erase(0, 2);
            else if (key.rfind('-', 0) == 0) key.erase(0, 1);
            else throw std::runtime_error("Лишние аргументы");
            if (key != "task" && key != "file" && key != "query" && key != "method") {
                throw std::runtime_error("Неизвестный аргумент: " + key);
            }
            options[key] = value;
        }
        auto mode = lab2::ProbeMode::Double;
        if (options.count("method")) {
            if (options["method"] == "linear") mode = lab2::ProbeMode::Linear;
            else if (options["method"] != "double") throw std::runtime_error("Метод: linear или double");
        }
        if (options.count("file") || options.count("query")) {
            if (options["file"].empty() || options["query"].empty() || options.count("task")) {
                throw std::runtime_error("Для множества задайте --file и --query без --task");
            }
            lab2::run_set(options["file"], options["query"], std::cout);
        } else if (options.count("task")) {
            lab2::run_task(options["task"], std::cin, std::cout, mode);
        } else {
            return interactive();
        }
    } catch (const std::exception& error) {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
