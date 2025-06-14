#pragma once

#include <cstddef>
#include <utility>
#include <stdexcept>
#include <vector>

template<typename Key, typename Value>
class HashMap {
public:
    HashMap(size_t size = 16);
    ~HashMap();

    void put(const Key& key, const Value& value);
    void remove(const Key& key);
    Value get(const Key& key) const;

private:
    struct Node {
        Key key;
        Value value;
        Node* next;
    };

    std::vector<Node*> table;
    size_t current_size;

    size_t hash(const Key& key) const;
    void clear();
};

template<typename Key, typename Value>
HashMap<Key, Value>::HashMap(size_t size) : table(size, nullptr), current_size(0) {}

template<typename Key, typename Value>
HashMap<Key, Value>::~HashMap() {
    clear();
}

template<typename Key, typename Value>
size_t HashMap<Key, Value>::hash(const Key& key) const {
    return std::hash<Key>()(key) % table.size();
}

template<typename Key, typename Value>
void HashMap<Key, Value>::put(const Key& key, const Value& value) {
    size_t index = hash(key);
    Node* newNode = new Node{key, value, nullptr};

    if (!table[index]) {
        table[index] = newNode;
    } else {
        Node* current = table[index];
        while (current->next) {
            if (current->key == key) {
                current->value = value; // Update existing key
                delete newNode;
                return;
            }
            current = current->next;
        }
        current->next = newNode;
    }
    current_size++;
}

template<typename Key, typename Value>
void HashMap<Key, Value>::remove(const Key& key) {
    size_t index = hash(key);
    Node* current = table[index];
    Node* prev = nullptr;

    while (current) {
        if (current->key == key) {
            if (prev) {
                prev->next = current->next;
            } else {
                table[index] = current->next;
            }
            delete current;
            current_size--;
            return;
        }
        prev = current;
        current = current->next;
    }
    throw std::runtime_error("Key not found");
}

template<typename Key, typename Value>
Value HashMap<Key, Value>::get(const Key& key) const {
    size_t index = hash(key);
    Node* current = table[index];

    while (current) {
        if (current->key == key) {
            return current->value;
        }
        current = current->next;
    }
    throw std::runtime_error("Key not found");
}

template<typename Key, typename Value>
void HashMap<Key, Value>::clear() {
    for (auto& head : table) {
        Node* current = head;
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
        head = nullptr;
    }
    current_size = 0;
}