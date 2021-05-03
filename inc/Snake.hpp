#pragma once

#include <string>

#include "SnakeBlocks.hpp"

namespace terminal {

template <typename T>
class Snake {
    class Iterator {
        Snake<T>& parent;
        typename SnakeBlocks<T>::Iterator blocks_iter;
        std::size_t elem_idx;

        Iterator(Snake<T>& parent, typename SnakeBlocks<T>::Iterator blocks_iter, std::size_t elem_idx);

        void increment();

    public:
        Iterator(Snake<T>& parent, std::size_t pos);
        Iterator& operator=(const Iterator&) = default;

        Iterator operator+(std::size_t offset) const;
        Iterator& operator++();
        Iterator operator++(int);

        T& operator*();

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;
    };

    static constexpr int BLOCK_SIZE = 4;
    SnakeBlocks<T> blocks;
    std::size_t head, tail;

public:
    Snake();

    inline std::size_t head_pos() { return head; }
    inline std::size_t tail_pos() { return tail; }
    inline std::size_t size() { return head - tail; }

    inline Iterator iter(std::size_t pos) { return Iterator(*this, pos); }
    inline Iterator begin() { return iter(tail); }
    inline Iterator end() { return iter(head); }
    inline Iterator begin(std::size_t offset) { return iter(tail + offset); }
    inline Iterator end(std::size_t offset) { return iter(head + offset); }

    void advance_head(std::size_t offset);
    void advance_tail(std::size_t offset);

    T& operator[](std::size_t index);

    void read_from_string(std::string s);
    void push(T value);
};

};
