#include "Reader.hpp"
#include "LineBuffer.hpp"
#include "StreamReader.hpp"
#include "TextRenderer.hpp"

namespace rl {
    extern "C" {
        #include "raylib.h"
    }
}

#include <fstream>
#include <iostream>

#ifndef SNAKE_BLOCKS_TEST_MAIN
bool init_line_buffer(terminal::LineBuffer& buffer) {
    std::ifstream file;
    file.open("display_data.txt");
    if (!file.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return false;
    }
    terminal::StreamReader reader(file);
    buffer.read_from(reader);
    file.close();
    return true;
}

int main() {
    int screenWidth = 640;
    int screenHeight = 480;

    int first_line = 0;
    float mouse_scroll_pos = 0.0F;

    rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // NOTE: textures and fonts must be loaded after window initialization (OpenGL context is required)
    terminal::TextRenderer renderer({0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight)});
    int num_lines = renderer.temporary_get_num_lines();
    terminal::LineBuffer lb(100, renderer.temporary_get_line_width());
    if (!init_line_buffer(lb)) {
        return 1;
    }

    rl::SetTargetFPS(30);

    while (!rl::WindowShouldClose()) {
        // handle window resize
        if (rl::IsWindowResized()) {
            screenWidth = rl::GetScreenWidth();
            screenHeight = rl::GetScreenHeight();
            renderer.set_viewport({0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight)});
            lb.set_line_width(renderer.temporary_get_line_width());
            num_lines = renderer.temporary_get_num_lines();
        }
        // update first displayed line number
        mouse_scroll_pos -= rl::GetMouseWheelMove();
        if (mouse_scroll_pos < 0.0F)
            mouse_scroll_pos = 0.0F;
        else if (mouse_scroll_pos + num_lines > lb.last_line_num())
            mouse_scroll_pos = lb.last_line_num() - num_lines;
        first_line = mouse_scroll_pos;

        // update line range
        auto line_range = lb.range(first_line, first_line + num_lines);
        renderer.set_line_range(line_range);

        terminal::LineBuffer::Line *line;
        int col;
        renderer.get_hovered_char(rl::GetMousePosition(), line, col);

        if (line && rl::IsMouseButtonDown(rl::MOUSE_BUTTON_LEFT)) {
            *(line->first_codepoint().resolve() + col) = '~';
        }

        // draw
        rl::BeginDrawing();
        rl::ClearBackground(rl::RAYWHITE);
        renderer.draw();
        rl::EndDrawing();
    }

    rl::CloseWindow();

    return 0;
}
#endif
