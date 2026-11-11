#include "tasks.h"
#include "support.h"
#include <algorithm>
#include <numeric>

namespace lab2 {

void run_task6(std::istream& input, std::ostream& output, ProbeMode mode) {
    const std::size_t count = read_count(input);
    HashTable<std::size_t> ids(count, mode);
    std::vector<std::string> names;
    std::vector<std::vector<std::size_t>> children;
    std::vector<std::size_t> parents;
    const auto no_parent = std::numeric_limits<std::size_t>::max();
    const auto id = [&](const std::string& name) {
        if (auto* found = ids.get(name)) return *found;
        const auto index = names.size();
        ids.put(name, index);
        names.push_back(name);
        children.emplace_back();
        parents.push_back(no_parent);
        return index;
    };
    for (std::size_t i = 0; i < count; ++i) {
        const std::string employee = read_word(input);
        const std::string manager = read_word(input);
        const auto a = id(employee);
        const auto b = id(manager);
        if (parents[a] != no_parent) {
            if (parents[a] == b) continue;
            throw std::runtime_error("У сотрудника несколько менеджеров");
        }
        parents[a] = b;
        if (a != b) children[b].push_back(a);
    }

    std::vector<unsigned char> state(names.size(), 0);
    std::vector<std::size_t> postorder;
    TaskStack stack;
    for (std::size_t start = 0; start < names.size(); ++start) {
        if (state[start] != 0) continue;
        stack.push(static_cast<std::int64_t>(start));
        while (!stack.empty()) {
            const auto code = stack.pop();
            if (code < 0) {
                const auto current = static_cast<std::size_t>(-code - 1);
                state[current] = 2;
                postorder.push_back(current);
                continue;
            }
            const auto current = static_cast<std::size_t>(code);
            if (state[current] == 2) continue;
            if (state[current] == 1) throw std::runtime_error("Цикл в иерархии сотрудников");
            state[current] = 1;
            stack.push(-code - 1);
            for (auto child : children[current]) {
                stack.push(static_cast<std::int64_t>(child));
            }
        }
    }

    std::vector<std::vector<std::string>> subordinates(names.size());
    for (auto current : postorder) {
        for (auto child : children[current]) {
            subordinates[current].push_back(names[child]);
            subordinates[current].insert(subordinates[current].end(),
                                         subordinates[child].begin(), subordinates[child].end());
        }
        std::sort(subordinates[current].begin(), subordinates[current].end());
    }
    std::vector<std::size_t> order(names.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](auto a, auto b) { return names[a] < names[b]; });
    for (auto current : order) {
        output << names[current] << '-' << subordinates[current].size();
        if (!subordinates[current].empty()) output << ':';
        for (const auto& name : subordinates[current]) output << ' ' << name;
        output << '\n';
    }
}

} // namespace lab2
