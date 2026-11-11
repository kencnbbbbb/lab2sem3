#include "tasks.h"
#include "support.h"

namespace lab2 {

void run_task3(std::istream& input, std::ostream& output) {
    const auto first = read_word(input);
    const auto second = read_word(input);
    for (const auto* genome : {&first, &second}) {
        if (genome->empty() || genome->size() > 100000) {
            throw std::runtime_error("Длина генома должна быть от 1 до 100000");
        }
        for (char base : *genome) {
            if (base < 'A' || base > 'Z') throw std::runtime_error("Геном должен содержать A..Z");
        }
    }
    Set pairs(1024);
    for (std::size_t i = 0; i + 1 < second.size(); ++i) {
        pairs.add(second.substr(i, 2));
    }
    std::size_t result = 0;
    for (std::size_t i = 0; i + 1 < first.size(); ++i) {
        result += pairs.contains(first.substr(i, 2));
    }
    output << result << '\n';
}

} // namespace lab2
