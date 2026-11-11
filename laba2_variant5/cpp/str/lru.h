#ifndef LAB2_LRU_H
#define LAB2_LRU_H

#include "linked_list.h"
#include "open_hash.h"
#include <memory>

namespace lab2 {

class Lru {
    struct CacheEntry {
        Node* node;
        std::string value;
    };

    std::size_t capacity_;
    LinkedList list_ = Linked_create();
    HashTable<CacheEntry> table_;

    void detach(Node* node) {
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            list_.head = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            list_.tail = node->prev;
        }
        node->next = nullptr;
        node->prev = nullptr;
    }

    void front(Node* node) {
        node->next = list_.head;
        if (list_.head) {
            list_.head->prev = node;
        } else {
            list_.tail = node;
        }
        list_.head = node;
    }

public:
    explicit Lru(std::size_t capacity) : capacity_(capacity) {}
    ~Lru() { Linked_destroy(list_); }
    Lru(const Lru&) = delete;
    Lru& operator=(const Lru&) = delete;

    std::string get(const std::string& key) {
        CacheEntry* entry = table_.get(key);
        if (!entry) {
            return "-1";
        }
        detach(entry->node);
        front(entry->node);
        return entry->value;
    }

    void set(const std::string& key, const std::string& value) {
        if (capacity_ == 0) {
            return;
        }
        if (CacheEntry* entry = table_.get(key)) {
            entry->value = value;
            detach(entry->node);
            front(entry->node);
            return;
        }
        auto node = std::make_unique<Node>(Node{key, nullptr, nullptr});
        table_.put(key, CacheEntry{node.get(), value});
        front(node.release());
        if (table_.size() > capacity_) {
            table_.erase(list_.tail->data);
            Linked_pop_back(list_);
        }
    }
};

} // namespace lab2
#endif
