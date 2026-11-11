#include "../tasks/tasks.h"
#include "../tasks/support.h"
#include "../str/array.h"
#include "../str/lru.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {
std::size_t assertions = 0;
std::size_t groups = 0;

void check(bool value, const char* description, int line) {
    ++assertions;
    if (!value) {
        throw std::runtime_error(std::string(description) + " at line " + std::to_string(line));
    }
}
#define CHECK(value) check((value), #value, __LINE__)

void group(const std::string& name, const std::function<void()>& body) {
    body();
    ++groups;
    std::cout << "PASS " << name << '\n';
}

std::string run(const std::string& task, const std::string& data,
                lab2::ProbeMode mode = lab2::ProbeMode::Double) {
    std::istringstream input(data);
    std::ostringstream output;
    lab2::run_task(task, input, output, mode);
    return output.str();
}

bool rejects(const std::string& task, const std::string& data) {
    try {
        run(task, data);
        return false;
    } catch (const std::exception&) {
        return true;
    }
}

void hash_random(lab2::ProbeMode mode) {
    lab2::HashTable<int> table(16, mode);
    std::unordered_map<std::string, int> reference;
    std::mt19937 rng(5);
    for (int i = 0; i < 20000; ++i) {
        const auto key = "key_" + std::to_string(rng() % 1000);
        switch (rng() % 3) {
            case 0:
                reference[key] = i;
                table.put(key, i);
                break;
            case 1:
                CHECK(table.erase(key) == (reference.erase(key) != 0));
                break;
            case 2: {
                const auto found = reference.find(key);
                const auto* value = table.get(key);
                CHECK((value != nullptr) == (found != reference.end()));
                if (value) CHECK(*value == found->second);
                break;
            }
        }
        CHECK(table.size() == reference.size());
    }
    for (const auto& pair : reference) {
        CHECK(table.get(pair.first) && *table.get(pair.first) == pair.second);
    }
    table.put("", 42);
    CHECK(table.get("") && *table.get("") == 42);
    const auto capacity = table.capacity();
    table.put("", 7);
    CHECK(table.capacity() == capacity && *table.get("") == 7);
}

void lru_random() {
    for (std::size_t capacity : {0, 1, 2, 10}) {
        lab2::Lru cache(capacity);
        std::map<std::string, std::string> reference;
        std::vector<std::string> order;
        std::mt19937 rng(51);
        const auto touch = [&](const std::string& key) {
            order.erase(std::remove(order.begin(), order.end(), key), order.end());
            order.insert(order.begin(), key);
        };
        for (int i = 0; i < 3000; ++i) {
            const auto key = std::to_string(rng() % 20);
            if (rng() % 2) {
                const auto value = std::to_string(i);
                cache.set(key, value);
                if (capacity != 0) {
                    reference[key] = value;
                    touch(key);
                    if (order.size() > capacity) {
                        reference.erase(order.back());
                        order.pop_back();
                    }
                }
            } else {
                const auto found = reference.find(key);
                CHECK(cache.get(key) == (found == reference.end() ? "-1" : found->second));
                if (found != reference.end()) touch(key);
            }
        }
    }
}

} // namespace

int main() {
    try {
        group("hash linear against independent map", [] { hash_random(lab2::ProbeMode::Linear); });
        group("hash double against independent map", [] { hash_random(lab2::ProbeMode::Double); });
        group("hash collision cluster and tombstones", [] {
            for (auto mode : {lab2::ProbeMode::Linear, lab2::ProbeMode::Double}) {
                lab2::HashTable<int> table(512, mode);
                std::vector<std::string> keys;
                for (int i = 0; keys.size() < 200; ++i) {
                    const auto key = "collision_" + std::to_string(i);
                    if (lab2::hash_key(key) % 512 == 0) keys.push_back(key);
                }
                for (int i = 0; i < 200; ++i) table.put(keys[i], i);
                for (int i = 0; i < 100; ++i) CHECK(table.erase(keys[i]));
                for (int i = 100; i < 200; ++i) CHECK(table.get(keys[i]) && *table.get(keys[i]) == i);
                for (int i = 0; i < 100; ++i) table.put(keys[i], -i);
                CHECK(table.size() == 200);
                for (int i = 0; i < 100; ++i) CHECK(*table.get(keys[i]) == -i);
            }
        });
        group("set duplicate and empty keys", [] {
            lab2::HashTable<std::string> alias;
            for (int i = 0; i < 11; ++i) alias.put(std::to_string(i), "value_" + std::to_string(i));
            const auto* value = alias.get("0");
            alias.put("copied", *value);
            CHECK(alias.capacity() == 32);
            CHECK(*alias.get("copied") == "value_0");
            lab2::Set set;
            set.add("a"); set.add("a"); set.add("");
            CHECK(set.size() == 2);
            CHECK(set.contains(""));
            CHECK(set.erase("a"));
            CHECK(!set.erase("a"));
            CHECK(set.size() == 1);
        });
        group("LR1 stack and expandable array", [] {
            lab2::TaskStack stack;
            stack.push(1); stack.push(2);
            CHECK(stack.pop() == 2);
            CHECK(stack.pop() == 1 && stack.empty());
            Array array = Array_create(16);
            for (int i = 0; i < 70000; ++i) Array_push_back(array, std::to_string(i));
            CHECK(Array_length(array) == 70000);
            CHECK(Array_get(array, 69999) == "69999");
            Array_destroy(array);
        });
        group("task1 DAG cycles and invalid input", [] {
            CHECK(run("1", "3 2 A B C A B B C") == "Возможно\nC B A\n");
            CHECK(run("1", "2 2 A B A B B A") == "Невозможно\n");
            CHECK(run("1", "1 1 A A A") == "Невозможно\n");
            CHECK(run("1", "0 0") == "Возможно\n\n");
            CHECK(rejects("1", "1 1 A A B"));
            CHECK(rejects("1", "2 0 A A"));
        });
        group("task3 genome pairs multiplicity and bounds", [] {
            CHECK(run("3", "ABBACAB BCABB") == "4\n");
            CHECK(run("3", "AAAA AA") == "3\n");
            CHECK(run("3", "A A") == "0\n");
            CHECK(run("3", std::string(100000, 'A') + " AA") == "99999\n");
            CHECK(rejects("3", "Aa AA"));
        });
        group("task4 dictionary and Unicode", [] {
            CHECK(run("4", "4 cAnnot cannOt fOund pAge thE pAge cAnnot be fouNd") == "2\n");
            CHECK(run("4", "4 cAnnot cannOt fOund pAge The PAGE cannot be found") == "4\n");
            CHECK(run("4", "0 тЕст ТЕст тест") == "2\n");
            CHECK(run("4", "1 тЕст тЕст теСт") == "1\n");
            CHECK(rejects("4", "1 word word"));
        });
        group("task5 recovery preserving node order", [] {
            CHECK(run("5", "1 2 # # 4 3 # # #") ==
                  "Исправленное дерево (preorder): 2 1 # # 4 3 # # #\nInorder: 1 2 3 4\n");
            CHECK(run("5", "1 # 3 # 2 # #") ==
                  "Исправленное дерево (preorder): 1 # 2 # 3 # #\nInorder: 1 2 3\n");
        });
        group("task5 malformed and non-repairable trees", [] {
            for (const auto* input : {"#", "1 # #", "2 2 # # 2 # #", "1 #", "1 # # 2"}) {
                CHECK(rejects("5", input));
            }
        });
        group("task6 hierarchy and both hash modes", [] {
            const std::string expected = "A-0\nB-2: A C\nC-0\nD-0\nE-4: A B C D\n";
            for (auto mode : {lab2::ProbeMode::Linear, lab2::ProbeMode::Double}) {
                CHECK(run("6.2", "5 A B C B D E B E E E", mode) == expected);
            }
            CHECK(rejects("6.2", "2 A B B A"));
            CHECK(rejects("6.2", "2 A B A C"));
            CHECK(run("6.2", "0").empty());
            CHECK(run("6.2", "2 A B A B") == "A-0\nB-1: A\n");
        });
        group("task7 examples integer keys and zero capacity", [] {
            CHECK(run("7", "2 8 SET 1 2 SET 2 3 SET 1 5 SET 4 5 SET 6 7 GET 4 SET 1 2 GET 3") == "5 -1\n");
            CHECK(run("7", "2 3 SET 01 7 GET 1 GET +01") == "7 7\n");
            CHECK(run("7", "0 2 SET 1 5 GET 1") == "-1\n");
            CHECK(rejects("7", "-1 0"));
            CHECK(rejects("7", "1 1 GET 9223372036854775808"));
            CHECK(rejects("7", "1 1 GET +-1"));
        });
        group("LRU randomized against independent ordered model", lru_random);
        std::cout << "PASS: " << groups << " groups, " << assertions << " assertions\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
