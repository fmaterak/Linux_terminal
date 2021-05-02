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
    struct Segment {
        // segment spans on [tail, head) -- these are indices of elements in `blocks`
        std::size_t tail, head;
    };

private:
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

    void advance_head();
    void advance_tail();

    T* operator[](std::size_t index);

#ifdef SNAKE_BLOCKS_TEST_MAIN
    friend void ::print_snake_blocks(SnakeBlocks<char>& snake);
    friend int ::main();
#endif
};

};
