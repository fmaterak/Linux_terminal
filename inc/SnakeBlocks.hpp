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
        // segment uses blocks [tail, head)
        std::size_t tail, head;
    };

private:
    std::size_t tail;  // reference point, snake only moves forward!
    std::size_t block_size;
    std::vector<T*> blocks;

    short state;
    Segment segments[3];

    void advance_seg_head_edge(Segment& segment);

    void advance_head();
    void advance_tail();

public:
    SnakeBlocks(std::size_t block_size, std::size_t initial_blocks = 0);

#ifdef SNAKE_BLOCKS_TEST_MAIN
    friend void ::print_snake_blocks(SnakeBlocks<char>& snake);
    friend int ::main();
#endif
};

};
