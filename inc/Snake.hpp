#pragma once

#include <string>

#include "SnakeBlocks.hpp"

namespace terminal {

template <typename T>
class Snake {
public:
    class Iterator {
        typename SnakeBlocks<T>::Iterator blocks_iter;
        std::size_t elem_idx;

        void increment();

    public:
        Iterator(SnakeBlocks<T>& blocks, std::size_t pos);
        Iterator& operator=(const Iterator&) = default;

        Iterator& operator++();
        Iterator operator++(int);

        T& operator*();

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;

        Iterator& resolve();
        std::size_t pos() const;
    };

private:
    static constexpr int BLOCK_SIZE = 1024;
    SnakeBlocks<T> blocks;
    std::size_t head, tail;

public:
    Snake();

    inline std::size_t head_pos() { return head; }
    inline std::size_t tail_pos() { return tail; }
    inline std::size_t size() { return head - tail; }

    inline Iterator iter(std::size_t pos) { return Iterator(blocks, pos); }
    inline Iterator begin() { return iter(tail); }
    inline Iterator end() { return iter(head); }
    inline Iterator begin(std::size_t offset) { return iter(tail + offset); }
    inline Iterator end(std::size_t offset) { return iter(head + offset); }

    void advance_head(std::size_t offset);
    void discard_head(std::size_t offset);
    void advance_tail(std::size_t offset);

    inline Iterator make_writing_region(std::size_t size) {
        advance_head(size);
        return end(-size);
    }

    T& operator[](std::size_t index);

    void read_from_string(std::string s);
    void push(T value);
};

};
