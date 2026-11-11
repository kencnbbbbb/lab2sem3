#include "tasks.h"
#include "support.h"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <unistd.h>

namespace lab2 {
namespace {

using SetData = std::map<std::string, std::vector<std::string>>;

class JsonSets {
    const std::string& text_;
    std::size_t position_ = 0;

    void whitespace() {
        while (position_ < text_.size() &&
               (text_[position_] == ' ' || text_[position_] == '\n' ||
                text_[position_] == '\r' || text_[position_] == '\t')) {
            ++position_;
        }
    }

    bool accept(char ch) {
        whitespace();
        if (position_ < text_.size() && text_[position_] == ch) {
            ++position_;
            return true;
        }
        return false;
    }

    void expect(char ch) {
        if (!accept(ch)) {
            throw std::runtime_error("Повреждён файл данных JSON");
        }
    }

    std::uint32_t hex4() {
        std::uint32_t code = 0;
        for (unsigned i = 0; i < 4; ++i) {
            if (position_ == text_.size()) {
                throw std::runtime_error("Неполная Unicode-последовательность JSON");
            }
            const char ch = text_[position_++];
            unsigned digit;
            if (ch >= '0' && ch <= '9') digit = static_cast<unsigned>(ch - '0');
            else if (ch >= 'a' && ch <= 'f') digit = static_cast<unsigned>(ch - 'a' + 10);
            else if (ch >= 'A' && ch <= 'F') digit = static_cast<unsigned>(ch - 'A' + 10);
            else throw std::runtime_error("Неверная Unicode-последовательность JSON");
            code = code * 16 + digit;
        }
        return code;
    }

    std::string string() {
        expect('"');
        std::string result;
        while (position_ < text_.size()) {
            const unsigned char ch = text_[position_++];
            if (ch == '"') {
                utf8_decode(result);
                return result;
            }
            if (ch < 0x20) {
                throw std::runtime_error("Управляющий символ в JSON-строке");
            }
            if (ch != '\\') {
                result.push_back(static_cast<char>(ch));
                continue;
            }
            if (position_ == text_.size()) {
                break;
            }
            const char escape = text_[position_++];
            switch (escape) {
                case '"': result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                case 'u': {
                    std::uint32_t code = hex4();
                    if (code >= 0xD800 && code <= 0xDBFF) {
                        if (position_ + 2 > text_.size() ||
                            text_[position_] != '\\' || text_[position_ + 1] != 'u') {
                            throw std::runtime_error("Неполная суррогатная пара JSON");
                        }
                        position_ += 2;
                        const auto low = hex4();
                        if (low < 0xDC00 || low > 0xDFFF) {
                            throw std::runtime_error("Неверная суррогатная пара JSON");
                        }
                        code = 0x10000 + ((code - 0xD800) << 10) + low - 0xDC00;
                    } else if (code >= 0xDC00 && code <= 0xDFFF) {
                        throw std::runtime_error("Неверная суррогатная пара JSON");
                    }
                    result += utf8_encode(code);
                    break;
                }
                default: throw std::runtime_error("Неверное экранирование JSON");
            }
        }
        throw std::runtime_error("Незавершённая JSON-строка");
    }

    std::vector<std::string> array() {
        whitespace();
        if (text_.compare(position_, 4, "null") == 0) {
            position_ += 4;
            return {};
        }
        expect('[');
        std::vector<std::string> result;
        if (accept(']')) return result;
        do {
            result.push_back(string());
        } while (accept(','));
        expect(']');
        return result;
    }

public:
    explicit JsonSets(const std::string& text) : text_(text) {}

    SetData parse() {
        SetData result;
        expect('{');
        if (!accept('}')) {
            do {
                const std::string name = string();
                expect(':');
                result[name] = array();
            } while (accept(','));
            expect('}');
        }
        whitespace();
        if (position_ != text_.size()) {
            throw std::runtime_error("Лишние данные после JSON-объекта");
        }
        return result;
    }
};

std::string quote(const std::string& text) {
    static const char hex[] = "0123456789abcdef";
    std::string result = "\"";
    for (unsigned char ch : text) {
        switch (ch) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:
                if (ch < 0x20) {
                    result += "\\u00";
                    result += hex[ch >> 4];
                    result += hex[ch & 15];
                } else {
                    result.push_back(static_cast<char>(ch));
                }
        }
    }
    return result + '"';
}

std::string encode(const SetData& data) {
    std::ostringstream output;
    output << "{\n";
    bool first_set = true;
    for (const auto& pair : data) {
        if (!first_set) output << ",\n";
        first_set = false;
        output << "  " << quote(pair.first) << ": [";
        for (std::size_t i = 0; i < pair.second.size(); ++i) {
            if (i) output << ", ";
            output << quote(pair.second[i]);
        }
        output << ']';
    }
    output << "\n}\n";
    return output.str();
}

void replace_file(const std::filesystem::path& filename, const std::string& content) {
    auto parent = filename.parent_path();
    if (parent.empty()) parent = ".";
    std::string pattern = (parent / ".laba2-cpp-XXXXXX").string();
    std::vector<char> temp_name(pattern.begin(), pattern.end());
    temp_name.push_back('\0');
    const int descriptor = mkstemp(temp_name.data());
    if (descriptor < 0) {
        throw std::runtime_error("Не удалось создать временный файл: " + std::string(std::strerror(errno)));
    }
    bool closed = false;
    try {
        std::size_t written = 0;
        while (written < content.size()) {
            const auto count = write(descriptor, content.data() + written, content.size() - written);
            if (count < 0 && errno == EINTR) continue;
            if (count <= 0) throw std::runtime_error("Ошибка записи файла");
            written += static_cast<std::size_t>(count);
        }
        if (fsync(descriptor) != 0) throw std::runtime_error("Ошибка сохранения файла");
        const int close_result = close(descriptor);
        closed = true;
        if (close_result != 0) throw std::runtime_error("Ошибка закрытия файла");
        std::filesystem::rename(temp_name.data(), filename);
    } catch (...) {
        if (!closed) close(descriptor);
        std::error_code ignored;
        std::filesystem::remove(temp_name.data(), ignored);
        throw;
    }
}

} // namespace

void run_set(const std::string& filename, const std::string& query, std::ostream& output) {
    std::istringstream commands(query);
    const std::string operation = read_word(commands);
    const std::string name = read_word(commands);
    std::vector<std::string> arguments;
    for (std::string value; commands >> value;) arguments.push_back(value);
    if (operation != "SETADD" && operation != "SETDEL" && operation != "SET_AT" && operation != "PRINT") {
        throw std::runtime_error("Неизвестная операция множества");
    }
    if (((operation == "SETADD" || operation == "SETDEL") && arguments.empty()) ||
        (operation == "SET_AT" && arguments.size() != 1) ||
        (operation == "PRINT" && !arguments.empty())) {
        throw std::runtime_error("Неверное число аргументов запроса");
    }

    SetData data;
    if (std::filesystem::exists(filename)) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) throw std::runtime_error("Не удалось открыть файл данных");
        const std::string raw{std::istreambuf_iterator<char>(file), {}};
        if (file.bad()) throw std::runtime_error("Ошибка чтения файла");
        data = JsonSets(raw).parse();
    }
    const auto found = data.find(name);
    Set set(found == data.end() ? 16 : found->second.size() * 2);
    if (found != data.end()) {
        for (const auto& value : found->second) set.add(value);
    }
    if (operation == "SET_AT") {
        output << (set.contains(arguments[0]) ? "true\n" : "false\n");
        return;
    }
    if (operation == "PRINT") {
        auto values = set.keys();
        std::sort(values.begin(), values.end());
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i) output << ' ';
            output << values[i];
        }
        output << '\n';
        return;
    }
    for (const auto& value : arguments) {
        utf8_decode(value);
        if (operation == "SETADD") set.add(value);
        else set.erase(value);
    }
    utf8_decode(name);
    auto values = set.keys();
    std::sort(values.begin(), values.end());
    data[name] = std::move(values);
    replace_file(filename, encode(data));
}

} // namespace lab2
