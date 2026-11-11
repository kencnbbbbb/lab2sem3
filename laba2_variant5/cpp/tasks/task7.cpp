#include "tasks.h"
#include "support.h"
#include "../str/lru.h"

namespace lab2 {

void run_task7(std::istream& input, std::ostream& output) {
    const auto capacity = read_count(input);
    const auto queries = read_count(input);
    Lru cache(capacity);
    std::vector<std::string> results;
    for (std::size_t i = 0; i < queries; ++i) {
        const std::string operation = read_word(input);
        const auto key = std::to_string(integer(read_word(input)));
        if (operation == "GET") {
            results.push_back(cache.get(key));
        } else if (operation == "SET") {
            const auto value = std::to_string(integer(read_word(input)));
            cache.set(key, value);
        } else {
            throw std::runtime_error("Неизвестная операция кэша");
        }
    }
    for (std::size_t i = 0; i < results.size(); ++i) {
        if (i) output << ' ';
        output << results[i];
    }
    output << '\n';
}

void run_task(const std::string& task, std::istream& input, std::ostream& output, ProbeMode mode) {
    if (task == "1") return run_task1(input, output);
    if (task == "3") return run_task3(input, output);
    if (task == "4") return run_task4(input, output);
    if (task == "5") return run_task5(input, output);
    if (task == "6" || task == "6.2") return run_task6(input, output, mode);
    if (task == "6.1") return hash_experiment(output);
    if (task == "7") return run_task7(input, output);
    throw std::runtime_error("Неизвестное задание: " + task);
}

} // namespace lab2
