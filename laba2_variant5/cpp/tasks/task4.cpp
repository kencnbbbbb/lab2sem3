#include "tasks.h"
#include "support.h"
#include "../str/array.h"

namespace lab2 {

void run_task4(std::istream& input, std::ostream& output) {
    const std::size_t n = read_count(input);
    // Storage and operations come from the user's Lab 1 array.
    Array dictionary = Array_create(n);
    try {
        Set known(n);
        Set correct(n);
        for (std::size_t i = 0; i < n; ++i) {
            std::string word = read_word(input);
            if (uppercase_count(word) != 1) {
                throw std::runtime_error("В словарном слове должно быть одно ударение");
            }
            Array_push_back(dictionary, word);
        }
        for (std::size_t i = 0; i < Array_length(dictionary); ++i) {
            const std::string word = Array_get(dictionary, i);
            known.add(lower_word(word));
            correct.add(word);
        }
        std::size_t errors = 0;
        for (std::string word; input >> word;) {
            if (uppercase_count(word) != 1 || (known.contains(lower_word(word)) && !correct.contains(word))) {
                ++errors;
            }
        }
        if (input.bad()) throw std::runtime_error("Ошибка чтения текста");
        output << errors << '\n';
    } catch (...) {
        Array_destroy(dictionary);
        throw;
    }
    Array_destroy(dictionary);
}

} // namespace lab2
