#include "Snake.hpp"

template<typename T>
terminal::Snake<T>::Iterator::Iterator(SnakeBlocks<T>& blocks, std::size_t pos):
    blocks_iter(blocks.iter(pos / BLOCK_SIZE)), elem_idx(pos % BLOCK_SIZE) { }

template<typename T>
terminal::Snake<T>::Iterator::Iterator(typename SnakeBlocks<T>::Iterator blocks_iter, std::size_t elem_idx):
    blocks_iter(blocks_iter), elem_idx(elem_idx) { }

template<typename T>
typename terminal::Snake<T>::Iterator& terminal::Snake<T>::Iterator::operator++() {
    increment();
    return *this;
}

template<typename T>
typename terminal::Snake<T>::Iterator terminal::Snake<T>::Iterator::operator++(int) {
    auto tmp = *this;
    increment();
    return tmp;
}

template<typename T>
typename terminal::Snake<T>::Iterator terminal::Snake<T>::Iterator::operator+(std::ptrdiff_t offset) const {
    auto r = std::div(elem_idx + offset, BLOCK_SIZE);
    return Iterator(blocks_iter + r.quot, r.rem);
}

template<typename T>
T& terminal::Snake<T>::Iterator::operator*() {
    return (*blocks_iter)[elem_idx];
}

template<typename T>
bool terminal::Snake<T>::Iterator::operator==(const terminal::Snake<T>::Iterator& other) const {
    return blocks_iter == other.blocks_iter && elem_idx == other.elem_idx;
}

template<typename T>
bool terminal::Snake<T>::Iterator::operator!=(const terminal::Snake<T>::Iterator& other) const {
    return !(*this == other);
}

template<typename T>
std::size_t terminal::Snake<T>::Iterator::pos() const {
    return blocks_iter.pos() * BLOCK_SIZE + elem_idx;
}

template<typename T>
void terminal::Snake<T>::Iterator::increment() {
    elem_idx++;
    if (elem_idx == BLOCK_SIZE) {
        elem_idx = 0;
        blocks_iter++;
    }
}

template<typename T>
typename terminal::Snake<T>::Iterator& terminal::Snake<T>::Iterator::resolve() {
    blocks_iter.resolve();
    return *this;
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
void terminal::Snake<T>::discard_head(std::size_t offset) {
    head -= offset;  // TODO: wrong, block may be allocated multiple times
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
    auto mem_begin = make_writing_region(s.size());
    auto mem_end = end();
    auto s_begin = s.cbegin();
    auto s_end = s.cend();
    while (mem_begin != mem_end) {
        *(mem_begin++) = *(s_begin++);
    }
}

template<typename T>
void terminal::Snake<T>::push(T value) {
    advance_head(1);
    (*this)[head-1] = value;
}

template class terminal::Snake<char>;
template class terminal::Snake<int>;

// #include "LineBuffer.hpp"
// template class terminal::Snake<terminal::LineBuffer::CharAttr>;
