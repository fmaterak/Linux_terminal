#pragma once

#include "Snake.hpp"
#include "Style.hpp"
#include "LineBuffer.hpp"

namespace terminal {

struct Selection {
    LineBuffer::Line *line_a, *line_b;
    int col_a, col_b;  // column (char) number in a corresponding row (line)

    Selection();

    bool available() const;
    bool is_a_first() const;
    LineBuffer::Line* first_line() const;
    LineBuffer::Line* last_line() const;
    int first_col() const;
    int last_col() const;
};

};
