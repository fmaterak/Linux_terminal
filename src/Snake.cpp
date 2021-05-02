#include "Snake.hpp"

template<typename T>
terminal::Snake<T>::SnakeIterator::SnakeIterator(terminal::Snake<T>& snake, std::size_t pos): snake(snake) {
    auto d = std::div(pos, BLOCK_SIZE);
    block_idx = d.quot;
    elem_idx = d.rem;
    try {
        block = snake.blocks[block_idx];
    }
    catch (std::out_of_range) {
        block = nullptr;
    }
}

template<typename T>
typename terminal::Snake<T>::SnakeIterator terminal::Snake<T>::SnakeIterator::operator+(std::size_t offset) const {
    return SnakeIterator(snake, block_idx * BLOCK_SIZE + elem_idx + offset);
}

template<typename T>
typename terminal::Snake<T>::SnakeIterator& terminal::Snake<T>::SnakeIterator::operator++() {
    increment();
    return *this;
}

template<typename T>
typename terminal::Snake<T>::SnakeIterator terminal::Snake<T>::SnakeIterator::operator++(int) {
    auto tmp = *this;
    increment();
    return tmp;
}

template<typename T>
T& terminal::Snake<T>::SnakeIterator::operator*() {
    return block[elem_idx];
}

template<typename T>
const T& terminal::Snake<T>::SnakeIterator::operator*() const {
    return block[elem_idx];
}

template<typename T>
bool terminal::Snake<T>::SnakeIterator::operator==(const terminal::Snake<T>::SnakeIterator& other) const {
    return block_idx == other.block_idx && elem_idx == other.elem_idx;
}

template<typename T>
bool terminal::Snake<T>::SnakeIterator::operator!=(const terminal::Snake<T>::SnakeIterator& other) const {
    return !(*this == other);
}

template<typename T>
void terminal::Snake<T>::SnakeIterator::increment() {
    elem_idx++;
    if (elem_idx == BLOCK_SIZE) {
        elem_idx = 0;
        block_idx++;
        block = snake.blocks[block_idx];
    }
}



template<typename T>
terminal::Snake<T>::Snake(): head(0), tail(0), blocks(BLOCK_SIZE) { }

template<typename T>
void terminal::Snake<T>::advance_head(std::size_t offset) {
    std::size_t prev_head_block_idx;
    if (head == 0) {
        // no space used yet, occupy one block and pretend it is used
        blocks.advance_head();
        prev_head_block_idx = 0;
    } else {
        // head points to the first not used element (next after last used), hence -1
        prev_head_block_idx = (head - 1) / BLOCK_SIZE;
    }
    head += offset;
    std::size_t curr_head_block_idx = (head - 1) / BLOCK_SIZE;

    // allocate new blocks
    for (std::size_t i = curr_head_block_idx - prev_head_block_idx; i; i--) {
        blocks.advance_head();
    }
}

template<typename T>
void terminal::Snake<T>::advance_tail(std::size_t offset) {
    std::size_t prev_tail_block_idx = tail / BLOCK_SIZE;
    tail += offset;
    std::size_t curr_tail_block_idx = tail / BLOCK_SIZE;

    // free blocks that are no longer needed
    for (std::size_t i = curr_tail_block_idx - prev_tail_block_idx; i; i--) {
        blocks.advance_tail();
    }
}

template<typename T>
T& terminal::Snake<T>::operator[](std::size_t index) {
    if (index < tail || index >= head) {
        throw std::out_of_range("Snake::operator[]: index");
    }

    auto d = std::div(index, BLOCK_SIZE);
    return blocks[d.quot][d.rem];
}

template<>
void terminal::Snake<char>::read_from_string(std::string s) {
    advance_head(s.size());
    auto mem_begin = end(-s.size());
    auto mem_end = end();
    auto s_begin = s.cbegin();
    auto s_end = s.cend();
    while (mem_begin != mem_end) {
        *(mem_begin++) = *(s_begin++);
    }
}

template class terminal::Snake<char>;
