#include "SnakeBlocks.hpp"

template<typename T>
terminal::SnakeBlocks<T>::Iterator::Iterator(SnakeBlocks<T>& container, std::size_t block_idx):
    container(&container), block_idx(block_idx)
{
    try {
        resolve();
    }
    catch (std::out_of_range) {
        block = nullptr;
    }
}

template<typename T>
typename terminal::SnakeBlocks<T>::Iterator& terminal::SnakeBlocks<T>::Iterator::operator++() {
    increment();
    return *this;
}

template<typename T>
typename terminal::SnakeBlocks<T>::Iterator terminal::SnakeBlocks<T>::Iterator::operator++(int) {
    auto tmp = *this;
    increment();
    return tmp;
}

template<typename T>
void terminal::SnakeBlocks<T>::Iterator::increment() {
    block_idx++;
    try {
        resolve();
    }
    catch (std::out_of_range) {
        block = nullptr;
    }
}

template<typename T>
typename terminal::SnakeBlocks<T>::Iterator terminal::SnakeBlocks<T>::Iterator::operator+(std::ptrdiff_t offset) const {
    return Iterator(*container, block_idx + offset);
}

template<typename T>
T* terminal::SnakeBlocks<T>::Iterator::operator*() {
    return block;
}

template<typename T>
typename terminal::SnakeBlocks<T>::Iterator& terminal::SnakeBlocks<T>::Iterator::resolve() {
    block = (*container)[block_idx];
    return *this;
}



template<typename T>
terminal::SnakeBlocks<T>::SnakeBlocks(std::size_t block_size, std::size_t initial_blocks):
    block_size(block_size), tail(0), state(0)
{
    segments[0].head = 0;
    segments[0].tail = 0;

    blocks.reserve(initial_blocks);
    for (std::size_t i = 0; i < initial_blocks; i++) {
        blocks.push_back(new T[block_size]);
    }
}

// Advances segment head allocating new block if needed
template<typename T>
void terminal::SnakeBlocks<T>::advance_seg_head_edge(terminal::SnakeBlocks<T>::Segment& segment) {
    if (segment.head == blocks.size()) {
        blocks.push_back(new T[block_size]);
    }
    segment.head++;
}

template<typename T>
void terminal::SnakeBlocks<T>::advance_head() {
    if (state == 0) {
        // if segment's size is smaller than the number of preceding unused blocks
        if ((segments[0].head - segments[0].tail) < segments[0].tail) {
            // wrap around: enter state 1, create new segment at the beginning
            state = 1;
            segments[1].tail = 0;
            segments[1].head = 1;
        } else {
            // there is only one segment, which may reach end
            advance_seg_head_edge(segments[0]);
        }
    }
    else if (state == 1) {
        if (segments[1].head == segments[0].tail) {
            // segment head reached previous segment's tail: enter state 2, create new segment at the end
            state = 2;
            segments[2].head = segments[2].tail = segments[0].head;
            // segment 2 has size 0 at the moment and may reach end
            advance_seg_head_edge(segments[2]);
        } else {
            // segment's head is in the middle, do plain advance (no allocation needed)
            segments[1].head++;
        }
    }
    // for these two cases the segment being advanced is at the end
    else if (state == 2) { advance_seg_head_edge(segments[2]); }
    else if (state == 3) { advance_seg_head_edge(segments[1]); }
}

template<typename T>
void terminal::SnakeBlocks<T>::advance_tail() {
    // if the oldest segment is empty, do nothing
    if (segments[0].tail == segments[0].head) {
        return;
    }

    // advance the tail of the oldest segment and the reference point
    tail++;
    segments[0].tail++;

    // if the segment has been collapsed
    if (segments[0].tail == segments[0].head) {
        if (state == 0) {
            // move it back to the beginning
            segments[0].head = segments[0].tail = 0;
        }
        else if (state == 1) {
            // old segment is now gone, switch back to state 0
            state = 0;
            segments[0] = segments[1];
        }
        else if (state == 2) {
            // middle segment is now gone, switch to state 3 (the opposite of state 1)
            state = 3;
            segments[0] = segments[1];
            segments[1] = segments[2];
        }
        else if (state == 3) {
            // switch back to state 0
            state = 0;
            segments[0] = segments[1];
        }
    }
}

template<typename T>
T* terminal::SnakeBlocks<T>::operator[](std::size_t index) {
    index -= tail;

    if (index >= 0) {
        short num_seg = num_segments();

        for (short i = 0; i < num_seg; i++) {
            auto seg_size = segments[i].head - segments[i].tail;
            if (index < seg_size) {
                return blocks[segments[i].tail + index];
            } else {
                index -= seg_size;
            }
        }
    }

    throw std::out_of_range("SnakeBlocks::operator[]: block index");
}

template class terminal::SnakeBlocks<char>;
template class terminal::SnakeBlocks<int>;

// #include "LineBuffer.hpp"
// template class terminal::SnakeBlocks<terminal::LineBuffer::CharAttr>;

#ifdef SNAKE_BLOCKS_TEST_MAIN
    void print_snake_blocks(terminal::SnakeBlocks<char>& sb) {
        short num_segments = sb.num_segments();
        std::cout << "tail: " << sb.tail << " state: " << sb.state << " blocks: [";
        for (std::size_t i = 0; i < sb.blocks.size(); i++) {
            char segment = '.';
            for (short s = 0; s < num_segments; s++) {
                if (sb.segments[s].head > i && sb.segments[s].tail <= i) {
                    segment = '0' + s;
                    break;
                }
            }
            std::cout << segment;
        }
        std::cout << "]\n";
    }

    int main() {
        auto sb = terminal::SnakeBlocks<char>(1, 8);
        print_snake_blocks(sb);

        char action;

        while (true) {
            std::cout << "> ";
            std::cin >> action;
            if (action == 'h') {
                sb.advance_head();
                print_snake_blocks(sb);
            }
            else if (action == 't') {
                sb.advance_tail();
                print_snake_blocks(sb);
            }
            else if (action == 'q') {
                break;
            }
        }

        return 0;
    }
#endif

// std::size_t initial_blocks;

// if (initial_capacity == 0) {
//     initial_blocks = 1;
// } else {
//     initial_blocks = initial_capacity / block_size;
//     if (initial_capacity % block_size) {
//         initial_blocks++;
//     }
// }
