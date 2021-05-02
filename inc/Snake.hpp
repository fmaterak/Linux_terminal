#pragma once

#include <string>

#include "SnakeBlocks.hpp"

namespace terminal {

template <typename T>
class Snake {
    class SnakeIterator {
        Snake<T>& snake;
        T* block;
        std::size_t block_idx, elem_idx;

        void increment();

    public:
        SnakeIterator(Snake<T>& snake, std::size_t pos);

        SnakeIterator operator+(std::size_t offset) const;
        SnakeIterator& operator++();
        SnakeIterator operator++(int);

        T& operator*();
        const T& operator*() const;

        bool operator==(const SnakeIterator& other) const;
        bool operator!=(const SnakeIterator& other) const;
    };

    static constexpr int BLOCK_SIZE = 4;
    SnakeBlocks<T> blocks;
    std::size_t head, tail;

public:
    Snake();

    inline std::size_t head_pos() { return head; }
    inline std::size_t tail_pos() { return tail; }
    inline std::size_t size() { return head - tail; }

    inline SnakeIterator iter(std::size_t pos) { return SnakeIterator(*this, pos); }
    inline SnakeIterator begin() { return iter(tail); }
    inline SnakeIterator end() { return iter(head); }
    inline SnakeIterator begin(std::size_t offset) { return iter(tail + offset); }
    inline SnakeIterator end(std::size_t offset) { return iter(head + offset); }

    void advance_head(std::size_t offset);
    void advance_tail(std::size_t offset);

    T& operator[](std::size_t index);

    void read_from_string(std::string s);
};

};
