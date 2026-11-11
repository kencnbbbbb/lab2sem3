#ifndef LAB2_SUPPORT_H
#define LAB2_SUPPORT_H

#include "../str/stack.h"
#include <charconv>
#include <cstdint>
#include <istream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace lab2 {

inline std::string read_word(std::istream& input) {
    std::string word;
    if (!(input >> word)) {
        throw std::runtime_error("Неполные входные данные");
    }
    return word;
}

inline std::int64_t integer(const std::string& token) {
    if (token.empty()) {
        throw std::runtime_error("Ожидалось целое число");
    }
    const char* begin = token.data();
    const char* end = begin + token.size();
    if (*begin == '+') {
        ++begin;
        if (begin == end || *begin == '+' || *begin == '-') {
            throw std::runtime_error("Неверное целое число: " + token);
        }
    }
    std::int64_t result = 0;
    const auto parsed = std::from_chars(begin, end, result);
    if (parsed.ec != std::errc{} || parsed.ptr != end) {
        throw std::runtime_error("Неверное целое число: " + token);
    }
    return result;
}

inline std::size_t read_count(std::istream& input) {
    const std::int64_t value = integer(read_word(input));
    if (value < 0 || static_cast<std::uint64_t>(value) > std::numeric_limits<std::size_t>::max()) {
        throw std::runtime_error("Недопустимый размер");
    }
    return static_cast<std::size_t>(value);
}

// Owns the exact procedural Stack from the user's C++ Lab 1.
class TaskStack {
    Stack stack_ = Stack_create();

public:
    ~TaskStack() { Stack_destroy(stack_); }
    TaskStack() = default;
    TaskStack(const TaskStack&) = delete;
    TaskStack& operator=(const TaskStack&) = delete;
    bool empty() const { return stack_.list.head == nullptr; }
    void push(std::int64_t value) { Stack_push_back(stack_, std::to_string(value)); }
    std::int64_t pop() { return integer(Stack_pop(stack_)); }
};

std::vector<std::uint32_t> utf8_decode(const std::string& text);
std::string utf8_encode(std::uint32_t code);
std::string lower_word(const std::string& word);
std::size_t uppercase_count(const std::string& word);

} // namespace lab2
#endif
