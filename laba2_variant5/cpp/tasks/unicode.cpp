#include "support.h"
#include <locale>

namespace lab2 {

std::vector<std::uint32_t> utf8_decode(const std::string& text) {
    std::vector<std::uint32_t> result;
    for (std::size_t i = 0; i < text.size();) {
        const auto first = static_cast<unsigned char>(text[i++]);
        std::uint32_t code = first;
        unsigned remaining = 0;
        std::uint32_t minimum = 0;
        if (first >= 0xC2 && first <= 0xDF) {
            code &= 0x1F; remaining = 1; minimum = 0x80;
        } else if (first >= 0xE0 && first <= 0xEF) {
            code &= 0x0F; remaining = 2; minimum = 0x800;
        } else if (first >= 0xF0 && first <= 0xF4) {
            code &= 0x07; remaining = 3; minimum = 0x10000;
        } else if (first >= 0x80) {
            throw std::runtime_error("Неверная UTF-8 строка");
        }
        for (unsigned j = 0; j < remaining; ++j) {
            if (i == text.size()) {
                throw std::runtime_error("Неполная UTF-8 строка");
            }
            const auto next = static_cast<unsigned char>(text[i++]);
            if ((next & 0xC0) != 0x80) {
                throw std::runtime_error("Неверная UTF-8 строка");
            }
            code = (code << 6) | (next & 0x3F);
        }
        if (code < minimum || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) {
            throw std::runtime_error("Недопустимый Unicode-символ");
        }
        result.push_back(code);
    }
    return result;
}

std::string utf8_encode(std::uint32_t code) {
    std::string result;
    if (code <= 0x7F) {
        result.push_back(static_cast<char>(code));
    } else if (code <= 0x7FF) {
        result.push_back(static_cast<char>(0xC0 | (code >> 6)));
        result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else if (code <= 0xFFFF) {
        result.push_back(static_cast<char>(0xE0 | (code >> 12)));
        result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    } else {
        result.push_back(static_cast<char>(0xF0 | (code >> 18)));
        result.push_back(static_cast<char>(0x80 | ((code >> 12) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
        result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
    }
    return result;
}

static const std::ctype<wchar_t>& character_type() {
    static const std::locale unicode_locale = [] {
        for (const char* name : {"C.UTF-8", "en_US.UTF-8", ""}) {
            try {
                return std::locale(name);
            } catch (const std::runtime_error&) {
                // Try the next installed UTF-8 locale.
            }
        }
        return std::locale::classic();
    }();
    return std::use_facet<std::ctype<wchar_t>>(unicode_locale);
}

std::string lower_word(const std::string& word) {
    std::string result;
    for (auto code : utf8_decode(word)) {
        result += utf8_encode(static_cast<std::uint32_t>(character_type().tolower(static_cast<wchar_t>(code))));
    }
    return result;
}

std::size_t uppercase_count(const std::string& word) {
    std::size_t count = 0;
    for (auto code : utf8_decode(word)) {
        count += character_type().is(std::ctype_base::upper, static_cast<wchar_t>(code));
    }
    return count;
}

} // namespace lab2
