#pragma once

#include <vector>
#include <cstdlib>
#include <iostream>

// define this to compile an interactive demo
#ifdef SNAKE_BLOCKS_TEST_MAIN
    #include <iostream>

    namespace terminal {
        template <typename T>
        class SnakeBlocks;
    };

    void print_snake_blocks(terminal::SnakeBlocks<char>& snake);
    int main();
#endif



namespace terminal {

template <typename T>
class SnakeBlocks {
public:
    class Iterator {
        SnakeBlocks<T>& parent;
        T* block;
        std::size_t block_idx;

        void increment();
        void update_block_ptr();

    public:
        Iterator(SnakeBlocks<T>& parent, std::size_t block_idx);

        Iterator operator+(std::size_t offset) const;
        Iterator& operator++();
        Iterator operator++(int);

        T* operator*();

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;
    };

private:
    struct Segment {
        // segment spans on [tail, head) -- these are indices of elements in `blocks`
        std::size_t tail, head;
    };

    std::size_t tail;  // reference point for public API, snake only moves forward!
    std::size_t block_size;
    std::vector<T*> blocks;

    short state;
    Segment segments[3];

    inline short num_segments() {
        return state == 3 ? 2 : state + 1;
    }

    void advance_seg_head_edge(Segment& segment);

public:
    SnakeBlocks(std::size_t block_size, std::size_t initial_blocks = 0);

    inline std::size_t head_pos() { return tail + size(); }
    inline std::size_t tail_pos() { return tail; }
    inline std::size_t size() { return segments[num_segments()-1].head; }

    inline Iterator iter(std::size_t pos) { return Iterator(*this, pos); }
    inline Iterator begin() { return iter(tail_pos()); }
    inline Iterator end() { return iter(head_pos()); }
    inline Iterator begin(std::size_t offset) { return iter(tail_pos() + offset); }
    inline Iterator end(std::size_t offset) { return iter(head_pos() + offset); }

    void advance_head();
    void advance_tail();

    T* operator[](std::size_t index);

#ifdef SNAKE_BLOCKS_TEST_MAIN
    friend void ::print_snake_blocks(SnakeBlocks<char>& snake);
    friend int ::main();
#endif
};

};
