#ifndef LAB2_OPEN_HASH_H
#define LAB2_OPEN_HASH_H

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace lab2 {

enum class ProbeMode { Linear, Double };

inline std::uint64_t hash_key(const std::string& key) {
    std::uint64_t result = 5381;
    for (unsigned char byte : key) {
        result = result * 33 + byte;
    }
    return result;
}

inline std::uint64_t second_hash(const std::string& key) {
    std::uint64_t result = 14695981039346656037ULL;
    for (unsigned char byte : key) {
        result ^= byte;
        result *= 1099511628211ULL;
    }
    return result;
}

template <typename Value>
class HashTable {
    struct Entry {
        std::string key;
        Value value{};
        unsigned char state = 0;
    };

    struct Location {
        std::size_t index;
        bool found;
    };

    std::vector<Entry> slots_;
    std::size_t size_ = 0;
    std::size_t deleted_ = 0;
    ProbeMode mode_;
    static constexpr std::size_t missing = std::numeric_limits<std::size_t>::max();

    Location locate(const std::string& key) const {
        const std::size_t mask = slots_.size() - 1;
        std::size_t index = hash_key(key) & mask;
        const std::uint64_t step = mode_ == ProbeMode::Double ? second_hash(key) | 1ULL : 1;
        std::size_t first_deleted = missing;
        for (std::size_t visited = 0; visited < slots_.size(); ++visited) {
            ++probes;
            const Entry& entry = slots_[index];
            if (entry.state == 0) {
                return {first_deleted == missing ? index : first_deleted, false};
            }
            if (entry.state == 1 && entry.key == key) {
                return {index, true};
            }
            if (entry.state == 2 && first_deleted == missing) {
                first_deleted = index;
            }
            index = (index + step) & mask;
        }
        return {first_deleted, false};
    }

    static std::size_t doubled(std::size_t capacity) {
        if (capacity > std::numeric_limits<std::size_t>::max() / 2) {
            throw std::length_error("Слишком большая хеш-таблица");
        }
        return capacity * 2;
    }

    static bool at_threshold(std::size_t used, std::size_t capacity) {
        // ceil(0.7 * capacity), without overflowing the multiplication.
        const std::size_t limit = (capacity / 10) * 7 + ((capacity % 10) * 7 + 9) / 10;
        return used >= limit;
    }

    void resize(std::size_t capacity) {
        HashTable replacement(capacity, mode_);
        for (const Entry& entry : slots_) {
            if (entry.state == 1) {
                replacement.put(entry.key, entry.value);
            }
        }
        probes += replacement.probes;
        slots_.swap(replacement.slots_);
        size_ = replacement.size_;
        deleted_ = 0;
    }

public:
    mutable std::uint64_t probes = 0;

    explicit HashTable(std::size_t capacity = 16, ProbeMode mode = ProbeMode::Double)
        : mode_(mode) {
        std::size_t actual = 16;
        while (actual < capacity) {
            actual = doubled(actual);
        }
        slots_.resize(actual);
    }

    Value* get(const std::string& key) {
        const Location result = locate(key);
        return result.found ? &slots_[result.index].value : nullptr;
    }

    const Value* get(const std::string& key) const {
        const Location result = locate(key);
        return result.found ? &slots_[result.index].value : nullptr;
    }

    void put(const std::string& key, const Value& value) {
        Location result = locate(key);
        if (result.found) {
            slots_[result.index].value = value;
            return;
        }
        Entry candidate{key, value, 1};
        if (at_threshold(size_ + deleted_ + 1, slots_.size())) {
            const std::size_t capacity = at_threshold(size_ + 1, slots_.size())
                ? doubled(slots_.size()) : slots_.size();
            resize(capacity);
            result = locate(candidate.key);
        }
        if (result.index == missing) {
            resize(doubled(slots_.size()));
            result = locate(candidate.key);
        }
        const bool reused = slots_[result.index].state == 2;
        slots_[result.index] = std::move(candidate);
        deleted_ -= reused;
        ++size_;
    }

    bool erase(const std::string& key) {
        const Location result = locate(key);
        if (!result.found) {
            return false;
        }
        slots_[result.index] = Entry{};
        slots_[result.index].state = 2;
        --size_;
        ++deleted_;
        return true;
    }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return slots_.size(); }

    std::vector<std::string> keys() const {
        std::vector<std::string> result;
        result.reserve(size_);
        for (const Entry& entry : slots_) {
            if (entry.state == 1) {
                result.push_back(entry.key);
            }
        }
        return result;
    }
};

class Set {
    HashTable<unsigned char> table_;

public:
    explicit Set(std::size_t capacity = 16) : table_(capacity, ProbeMode::Double) {}
    void add(const std::string& value) { table_.put(value, 0); }
    bool erase(const std::string& value) { return table_.erase(value); }
    bool contains(const std::string& value) const { return table_.get(value) != nullptr; }
    std::vector<std::string> keys() const { return table_.keys(); }
    std::size_t size() const { return table_.size(); }
};

} // namespace lab2
#endif
