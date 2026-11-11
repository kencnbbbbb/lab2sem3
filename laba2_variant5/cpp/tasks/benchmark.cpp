#include "tasks.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace lab2 {
namespace {

class SplitMix64 {
    std::uint64_t state_ = 5;

public:
    std::uint64_t next() {
        std::uint64_t value = (state_ += 0x9E3779B97F4A7C15ULL);
        value = (value ^ (value >> 30)) * 0xBF58476D1CE4E5B9ULL;
        value = (value ^ (value >> 27)) * 0x94D049BB133111EBULL;
        return value ^ (value >> 31);
    }
};

std::vector<std::string> experiment_keys(std::size_t n, std::size_t capacity, bool collisions) {
    std::vector<std::string> keys;
    keys.reserve(n);
    SplitMix64 random;
    for (std::uint64_t i = 0; keys.size() < n; ++i) {
        if (collisions) {
            std::string key = "collision_" + std::to_string(i);
            if (hash_key(key) % capacity == 0) keys.push_back(std::move(key));
        } else {
            std::ostringstream key;
            key << 'k' << i << '_' << std::hex << std::setw(16) << std::setfill('0') << random.next();
            keys.push_back(key.str());
        }
    }
    return keys;
}

template <typename Operation>
std::int64_t measure(Operation operation) {
    const auto start = std::chrono::steady_clock::now();
    operation();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now() - start).count();
}

} // namespace

void hash_experiment(std::ostream& output) {
    output << "dataset,load,method,keys,insert_probes,hit_probes,miss_probes,delete_probes,"
              "insert_ns,hit_ns,miss_ns,delete_ns\n";
    for (bool collisions : {false, true}) {
        const std::size_t capacity = collisions ? 1024 : 16384;
        for (unsigned load : {25, 50, 65}) {
            const std::size_t count = capacity * load / 100;
            const auto keys = experiment_keys(count, capacity, collisions);
            // Both methods receive exactly the same keys and insertion order.
            for (ProbeMode mode : {ProbeMode::Linear, ProbeMode::Double}) {
                HashTable<std::size_t> table(capacity, mode);
                table.probes = 0;
                const auto insertion = measure([&] {
                    for (std::size_t i = 0; i < keys.size(); ++i) table.put(keys[i], i);
                });
                const auto insert_probes = table.probes;
                table.probes = 0;
                const auto hit = measure([&] {
                    for (std::size_t i = 0; i < keys.size(); ++i) {
                        const auto* value = table.get(keys[i]);
                        if (!value || *value != i) throw std::runtime_error("Потерян ключ");
                    }
                });
                const auto hit_probes = table.probes;
                table.probes = 0;
                const auto miss = measure([&] {
                    for (const auto& key : keys) {
                        if (table.get("missing_" + key)) throw std::runtime_error("Неожиданный ключ");
                    }
                });
                const auto miss_probes = table.probes;
                table.probes = 0;
                const auto removal = measure([&] {
                    for (const auto& key : keys) {
                        if (!table.erase(key)) throw std::runtime_error("Не удалён ключ");
                    }
                });
                const auto delete_probes = table.probes;
                output << (collisions ? "same_bucket" : "random") << ',' << load << ','
                       << (mode == ProbeMode::Linear ? "linear" : "double") << ',' << count << ','
                       << insert_probes << ',' << hit_probes << ',' << miss_probes << ',' << delete_probes << ','
                       << insertion << ',' << hit << ',' << miss << ',' << removal << '\n';
            }
        }
    }
}

} // namespace lab2
