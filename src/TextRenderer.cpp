#include "TextRenderer.hpp"

terminal::TextRenderer::TextRenderer(const Selection& selection, rl::Rectangle viewport): selection(selection) {
    static constexpr int font_size = 40;
    fonts[0] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-Regular.ttf", font_size, 0, 1120);
    fonts[1] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-Bold.ttf", font_size, 0, 1120);
    fonts[2] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-Italic.ttf", font_size, 0, 1120);
    fonts[3] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-BoldItalic.ttf", font_size, 0, 1120);
    rl::SetTextureFilter(fonts[0].texture, rl::TEXTURE_FILTER_BILINEAR);
    rl::SetTextureFilter(fonts[1].texture, rl::TEXTURE_FILTER_BILINEAR);
    rl::SetTextureFilter(fonts[2].texture, rl::TEXTURE_FILTER_BILINEAR);
    rl::SetTextureFilter(fonts[3].texture, rl::TEXTURE_FILTER_BILINEAR);
    char_height = font_size / 2;
    char_width = fonts[0].chars['X'].advanceX / 2;

    rlines = nullptr;
    set_viewport(viewport);
}

void terminal::TextRenderer::set_viewport(rl::Rectangle new_viewport) {
    viewport = new_viewport;
    num_cols = viewport.width / char_width;
    num_rows = viewport.height / char_height;
    origin.x = viewport.x + (viewport.width - num_cols*char_width) / 2;
    origin.y = viewport.y + (viewport.height - num_rows*char_height) / 2;
    if (rlines) delete rlines;
    rlines = new RenderableLine[num_rows];
}

void terminal::TextRenderer::set_line_range(terminal::LineBuffer::LineRange line_range) {
    auto next_line_iter = line_range.begin();
    auto last_line_iter = line_range.end();
    int i = 0;

    while (next_line_iter != last_line_iter) {
        LineBuffer::Line* line = &(*(next_line_iter++));
        int length = next_line_iter->first_codepoint().pos() - line->first_codepoint().pos();
        rlines[i++] = { line, length };
    }

    while (i < num_rows) {
        rlines[i++] = { nullptr, 0 };
    }
}

void terminal::TextRenderer::get_hovered_char(rl::Vector2 mouse_pos, terminal::LineBuffer::Line*& line, int& col) const {
    const auto& rline = rlines[static_cast<int>((mouse_pos.y - origin.y) / char_height)];
    line = rline.line;

    col = (mouse_pos.x - origin.x) / char_width;
    if (col >= rline.length) {
        col = rline.length - 1;
    }
}

rl::Vector2 terminal::TextRenderer::rowcol_to_vec(int row, int col) {
    return {
        origin.x + col*char_width,
        origin.y + row*char_height
    };
}

void terminal::TextRenderer::draw_styled(
    const terminal::Style& style, bool selected, terminal::Snake<terminal::codepoint>::Iterator& codepoints, int row, int col, int length)
{
    // if (length <= 0) {
    //     throw std::out_of_range("terminal::TextRenderer::draw_styled(): length <= 0");
    // }
    // else if (col < 0 || col >= num_cols) {
    //     throw std::out_of_range("terminal::TextRenderer::draw_styled(): col");
    // }
    // else if (row < 0 || row >= num_rows) {
    //     throw std::out_of_range("terminal::TextRenderer::draw_styled(): row");
    // }

    rl::Vector2 start = rowcol_to_vec(row, col);

    // colors
    rl::Color bg = rl::RAYWHITE;
    rl::Color fg = rl::BLACK;
    if (style.mask & Style::INVERTED) {
        std::swap(bg, fg);
    }

    // change bg for selected text
    if (selected) {
        rl::Color selection_color = rl::RED;
        bg.r = bg.r / 2 + selection_color.r / 2;
        bg.g = bg.g / 2 + selection_color.g / 2;
        bg.b = bg.b / 2 + selection_color.b / 2;
    }

    // font
    short index = (style.mask & Style::BOLD ? 1 : 0) + (style.mask & Style::ITALIC ? 2 : 0);
    const rl::Font& font = fonts[index];

    // draw background
    rl::DrawRectangleV(start, {char_width*length, char_height}, bg);

    // draw text
    rl::Vector2 pos = start;
    int chars_to_print = length;
    while (chars_to_print--) {
        codepoint c = *(codepoints++);
        rl::DrawTextCodepoint(font, c, pos, char_height, fg);
        pos.x += char_width;
    }

    // draw effects
    if (style.mask & Style::UNDERLINED) {
        rl::DrawRectangleV({start.x, start.y + char_height*0.85F}, {length*char_width + 2, 2}, fg);
    }
    if (style.mask & Style::STRIKEOUT) {
        rl::DrawRectangleV({start.x, start.y + char_height*0.55F}, {length*char_width + 2, 2}, fg);
    }
}

void terminal::TextRenderer::draw() {
    std::size_t next_style_change_pos;
    int col, length, sel_change_length;
    bool eol, in_selection = false, sel_available = selection.available();

    if (sel_available) {
        // if there is a selection somewhere, determine if the first visible line is selected
        in_selection =
            selection.first_line()->first_codepoint() < rlines[0].line->first_codepoint() &&
            selection.last_line()->first_codepoint() >= rlines[0].line->first_codepoint();
    }

    for (int row = 0; row < num_rows; row++) {
        const auto& rline = rlines[row];
        if (!rline.length) {
            continue;  // rline.line may be nullptr
        }

        const auto& line = *(rline.line);
        auto codepoint_iter = line.first_codepoint().resolve();
        auto next_style_change_iter = line.active_style();

        col = 0;
        eol = false;

        while (!eol) {
            auto style = (next_style_change_iter++)->new_style;
            next_style_change_pos = next_style_change_iter->effective_from;
            length = next_style_change_pos - codepoint_iter.pos();

            if (col + length >= rline.length) {
                length = rline.length - col;
                eol = true;
            }

            if (sel_available) {
                if (!in_selection &&
                    *selection.first_line() == line && selection.first_col() >= col && selection.first_col() < col + length)
                {
                    sel_change_length = selection.first_col() - col;
                    if (sel_change_length > 0) {
                        draw_styled(style, in_selection, codepoint_iter, row, col, sel_change_length);
                        col += sel_change_length;
                        length -= sel_change_length;
                    }
                    in_selection = true;
                }

                if (in_selection &&
                    *selection.last_line() == line && selection.last_col() >= col && selection.last_col() < col + length)
                {
                    sel_change_length = selection.last_col() - col + 1;
                    draw_styled(style, in_selection, codepoint_iter, row, col, sel_change_length);
                    col += sel_change_length;
                    length -= sel_change_length;
                    in_selection = false;
                }
            }

            if (length > 0) {
                draw_styled(style, in_selection, codepoint_iter, row, col, length);
                col += length;
            }
        }
    }
}
