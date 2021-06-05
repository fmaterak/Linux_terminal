#pragma once

#include "Style.hpp"
#include "LineBuffer.hpp"

namespace rl {
    extern "C" {
        #include "raylib.h"
    }
}

#include <stdexcept>

namespace terminal {

class TextRenderer {
    rl::Color fg;
    rl::Font fonts[4];
    rl::Rectangle viewport;
    rl::Vector2 origin;
    int num_rows, num_cols;
    float char_width, char_height;

    rl::Vector2 rowcol_to_vec(int row, int col);
    void draw_styled(const Style& style, Snake<codepoint>::Iterator& codepoints, int row, int col, int length);

public:
    TextRenderer(rl::Rectangle viewport);

    void set_viewport(rl::Rectangle new_viewport);
    void draw(LineBuffer::LineRange line_range);

    inline int temporary_get_line_width() const { return num_cols; }  // TODO: remove
    inline int temporary_get_num_lines() const { return num_rows; }  // TODO: remove
};

};
