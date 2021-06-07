#include "TextRenderer.hpp"

terminal::TextRenderer::TextRenderer(rl::Rectangle viewport) {
    static constexpr int font_size = 64;
    fonts[0] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-Regular.ttf", font_size, 0, 2048);
    fonts[1] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-Bold.ttf", font_size, 0, 2048);
    fonts[2] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-Italic.ttf", font_size, 0, 2048);
    fonts[3] = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-BoldItalic.ttf", font_size, 0, 2048);
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

void terminal::TextRenderer::get_hovered_char(rl::Vector2 mouse_pos, terminal::LineBuffer::Line*& line, int& col) {
    col = (mouse_pos.x - origin.x) / char_width;
    const auto& rline = rlines[static_cast<int>((mouse_pos.y - origin.y) / char_height)];
    line = (col < rline.length) ? rline.line : nullptr;
}

rl::Vector2 terminal::TextRenderer::rowcol_to_vec(int row, int col) {
    return {
        origin.x + col*char_width,
        origin.y + row*char_height
    };
}

void terminal::TextRenderer::draw_styled(
    const terminal::Style& style, terminal::Snake<terminal::codepoint>::Iterator& codepoints, int row, int col, int length)
{
    if (length <= 0) {
        throw std::out_of_range("terminal::TextRenderer::draw_styled(): length <= 0");
    }
    else if (col < 0 || col >= num_cols) {
        throw std::out_of_range("terminal::TextRenderer::draw_styled(): col");
    }
    else if (row < 0 || row >= num_rows) {
        throw std::out_of_range("terminal::TextRenderer::draw_styled(): row");
    }

    rl::Vector2 start = rowcol_to_vec(row, col);

    // colors
    rl::Color bg = rl::RAYWHITE;
    rl::Color fg = rl::BLACK;
    if (style.mask & Style::INVERTED) {
        std::swap(bg, fg);
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
    for (int row = 0; row < num_rows; row++) {
        const auto& rline = rlines[row];
        if (!rline.length) {
            continue;  // rline.line may be nullptr
        }

        const auto& line = *(rline.line);
        auto codepoint_iter = line.first_codepoint().resolve();
        auto next_style_change_iter = line.active_style();
        auto style = (next_style_change_iter++)->new_style;

        std::size_t next_style_change_pos, line_end = codepoint_iter.pos() + rline.length;
        int col = 0, length;

        // draw current line in all styles except the last one
        while ((next_style_change_pos = next_style_change_iter->effective_from) < line_end) {
            length = next_style_change_pos - codepoint_iter.pos();
            draw_styled(style, codepoint_iter, row, col, length);
            col += length;
            style = (next_style_change_iter++)->new_style;
        }

        // draw the rest of the line
        if ((length = line_end - codepoint_iter.pos())) {
            draw_styled(style, codepoint_iter, row, col, length);
        }
    }
}
