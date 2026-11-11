#include "tasks.h"
#include "support.h"
#include <utility>
#include <vector>

namespace lab2 {

void run_task1(std::istream& input, std::ostream& output) {
    const std::size_t n = read_count(input);
    const std::size_t m = read_count(input);
    HashTable<std::size_t> ids(n);
    std::vector<std::string> names;
    names.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
        std::string name = read_word(input);
        if (ids.get(name)) throw std::runtime_error("Повтор задачи");
        ids.put(name, i);
        names.push_back(std::move(name));
    }
    std::vector<std::vector<std::size_t>> dependencies(n);
    for (std::size_t i = 0; i < m; ++i) {
        const std::string a = read_word(input);
        const std::string b = read_word(input);
        const auto* first = ids.get(a);
        const auto* second = ids.get(b);
        if (!first || !second) throw std::runtime_error("Неизвестная задача");
        dependencies[*first].push_back(*second);
    }
    std::vector<unsigned char> color(n, 0);
    std::vector<std::size_t> order;
    TaskStack stack;
    for (std::size_t start = 0; start < n; ++start) {
        if (color[start] != 0) continue;
        stack.push(static_cast<std::int64_t>(start));
        while (!stack.empty()) {
            const auto code = stack.pop();
            if (code < 0) {
                const auto current = static_cast<std::size_t>(-code - 1);
                color[current] = 2;
                order.push_back(current);
                continue;
            }
            const auto current = static_cast<std::size_t>(code);
            if (color[current] == 2) continue;
            if (color[current] == 1) {
                output << "Невозможно\n";
                return;
            }
            color[current] = 1;
            stack.push(-code - 1);
            for (auto it = dependencies[current].rbegin(); it != dependencies[current].rend(); ++it) {
                stack.push(static_cast<std::int64_t>(*it));
            }
        }
    }
    output << "Возможно\n";
    for (std::size_t i = 0; i < order.size(); ++i) {
        if (i) output << ' ';
        output << names[order[i]];
    }
    output << '\n';
}

} // namespace lab2
