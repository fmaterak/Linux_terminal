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

#ifndef SNAKE_BLOCKS_TEST_MAIN

int main() {
    int screenWidth = 640;
    int screenHeight = 480;

    std::size_t first_line = 0;
    float mouse_scroll_pos = 0.0F;

    rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "terminal");

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

    rl::TraceLog(rl::LOG_INFO, rl::TextFormat("TERMINAL: child pid %d", shell.GetPid()));

    rl::SetTargetFPS(30);

    while (!rl::WindowShouldClose()) {
        // handle input
        int c, byte_len;

        if ((c = rl::GetCharPressed())) {
            const char* utf8 = rl::CodepointToUtf8(c, &byte_len);
            shell.write(utf8, byte_len);
        }

        if ((c = rl::GetKeyPressed())) {
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

        // autoscroll
        bool autoscroll = lb.lines_available() < num_lines || first_line + num_lines == lb.last_line_num();

        // get output
        lb.read_from(shell);
        if (!shell.is_connected()) {
            break;
        }

        if (autoscroll && lb.lines_available() > num_lines) {
            first_line = mouse_scroll_pos = lb.last_line_num() - num_lines;
        }

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
