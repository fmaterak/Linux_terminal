#include "Reader.hpp"
#include "Selection.hpp"
#include "LineBuffer.hpp"
#include "StreamReader.hpp"
#include "TextRenderer.hpp"
#include "SelectionProvider.hpp"
#include "Shell.hpp"

namespace rl {
    extern "C" {
        #include "raylib.h"
    }
}

#include <fstream>
#include <iostream>

#ifndef SNAKE_BLOCKS_TEST_MAIN
// bool init_line_buffer(terminal::LineBuffer& buffer) {
//     std::ifstream file;
//     file.open("display_data.txt");
//     if (!file.is_open()) {
//         std::cout << "Error opening file" << std::endl;
//         return false;
//     }
//     terminal::StreamReader reader(file);
//     buffer.read_from(reader);
//     file.close();
//     return true;
// }

int main() {
    int screenWidth = 640;
    int screenHeight = 480;

    std::size_t first_line = 0;
    float mouse_scroll_pos = 0.0F;

    rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    terminal::Selection selection;
    terminal::TextRenderer renderer(selection, {0, 0, static_cast<float>(screenWidth), static_cast<float>(screenHeight)});
    terminal::SelectionProvider selection_provider(selection, renderer);

    int num_lines = renderer.temporary_get_num_lines();
    terminal::LineBuffer lb(100, renderer.temporary_get_line_width());

    terminal::Shell shell;
    if (!shell.SpawnChild()) {
        rl::CloseWindow();
        return 1;
    }
    std::cout << "child pid: " << shell.GetPid() << std::endl;

    rl::SetTargetFPS(30);

    while (!rl::WindowShouldClose()) {
        lb.read_from(shell);

        int c, byte_len;

        if ((c = rl::GetCharPressed())) {
            const char* utf8 = rl::CodepointToUtf8(c, &byte_len);
            // std::cout << c << ": " << utf8 << std::endl;
            shell.write(utf8, byte_len);
        }

        if ((c = rl::GetKeyPressed())) {
            // std::cout << c << std::endl;
            if (c == rl::KEY_ENTER) {
                shell.write("\n", 1);
            }
        }


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
            mouse_scroll_pos = (lb.lines_available() > num_lines) ? lb.last_line_num() - num_lines : lb.first_line_num();
        first_line = mouse_scroll_pos;

        // update line range
        auto line_range = lb.range(first_line, std::min(first_line + num_lines, lb.last_line_num()));
        renderer.set_line_range(line_range);

        selection_provider.update();

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
