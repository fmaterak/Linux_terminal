#include "Selection.hpp"

terminal::Selection::Selection(): line_a(nullptr), line_b(nullptr) { }

bool terminal::Selection::available() const {
    return line_a && line_b;  // both pointers are non-null
}

bool terminal::Selection::is_a_first() const {
    const auto& line_a_start = line_a->first_codepoint();
    const auto& line_b_start = line_b->first_codepoint();
    return (line_a_start < line_b_start || (line_a_start == line_b_start && col_a <= col_b));
}

terminal::LineBuffer::Line* terminal::Selection::first_line() const {
    return is_a_first() ? line_a : line_b;
}

terminal::LineBuffer::Line* terminal::Selection::last_line() const {
    return is_a_first() ? line_b : line_a;
}

int terminal::Selection::first_col() const {
    return is_a_first() ? col_a : col_b;
}

int terminal::Selection::last_col() const {
    return is_a_first() ? col_b : col_a;
}
