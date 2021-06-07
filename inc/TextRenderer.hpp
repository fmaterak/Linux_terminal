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
public:
    struct RenderableLine {
        LineBuffer::Line* line;
        int length;
    };

private:
    rl::Color fg;
    rl::Font fonts[4];
    rl::Rectangle viewport;
    rl::Vector2 origin;
    int num_rows, num_cols;
    float char_width, char_height;
    int hovered_row, hovered_col;
    // LineBuffer::Line hovered_line;
    // Snake<codepoint>::Iterator hovered_char;
    RenderableLine *rlines;

    rl::Vector2 rowcol_to_vec(int row, int col);
    void draw_styled(const Style& style, Snake<codepoint>::Iterator& codepoints, int row, int col, int length);

public:
    TextRenderer(rl::Rectangle viewport);

    void set_viewport(rl::Rectangle new_viewport);
    void set_line_range(LineBuffer::LineRange line_range);
    // void get_hovered_char(rl::Vector2 mouse_pos);

    void draw();

    inline int temporary_get_line_width() const { return num_cols; }  // TODO: remove
    inline int temporary_get_num_lines() const { return num_rows; }  // TODO: remove
};

};
