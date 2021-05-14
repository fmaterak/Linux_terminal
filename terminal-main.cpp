#include <fstream>
#include <iostream>

namespace rl {
	extern "C" {
		#include "raylib.h"
	}
}

#include "LineBuffer.hpp"

#ifndef SNAKE_BLOCKS_TEST_MAIN
int main() {
    int screenWidth = 640;
    int screenHeight = 480;

    int display_lines = 5;
    int line_width = 35;
    int first_line = 0;

    terminal::LineBuffer lb(100, line_width);
    std::ifstream file;
    file.open("display_data.txt");
    if (!file.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return 1;
    }
    lb.read_from(file);
    file.close();

	rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // NOTE: extures and fonts must be loaded after window initialization (OpenGL context is required)
    rl::Font font = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-regular.ttf", 64, 0, 2048);

    // Setup texture scaling filter
    SetTextureFilter(font.texture, rl::TEXTURE_FILTER_BILINEAR);

    float fontSize = font.baseSize / 2;

    rl::SetTargetFPS(30);

    while (!rl::WindowShouldClose()) {
        int key;
        while ((key = rl::GetKeyPressed())) {
            if (key == rl::KEY_LEFT && line_width > 1) {
                line_width--;
                lb.set_line_width(line_width);
            }
            else if (key == rl::KEY_RIGHT && line_width < 1000) {
                line_width++;
                lb.set_line_width(line_width);
                // after rewrapping last_line_num may decrease
                if ((first_line + display_lines) > lb.last_line_num()) {
                    first_line = lb.last_line_num() - display_lines;
                }
            }
            else if (key == rl::KEY_UP && first_line > 0) {
                first_line--;
            }
            else if (key == rl::KEY_DOWN && (first_line + display_lines) < lb.last_line_num()) {
                first_line++;
            }
        }
        if (rl::IsWindowResized()) {
            screenWidth = rl::GetScreenWidth();
            screenHeight = rl::GetScreenHeight();
        }
        // int chr;
        // while ((chr = rl::GetCharPressed())) lb.push(chr);
        // if (rl::IsKeyPressed(rl::KEY_BACKSPACE)) snake.advance_tail(snake.size());
        rl::BeginDrawing();
        rl::ClearBackground(rl::RAYWHITE);
        // rl::DrawTextEx(font, msg1, { 40, (screenHeight - fontSize) / 2 - fontSize }, fontSize, 0, rl::BLACK);
        // rl::DrawTextEx(font, msg2, { 40, (screenHeight - fontSize) / 2 }, fontSize, 0, rl::BLACK);
        // rl::DrawTextEx(font, msg3, { 40, (screenHeight - fontSize) / 2 + fontSize}, fontSize, 0, rl::BLACK);

        int X0 = 40;
        int Y0 = (screenHeight - fontSize) / 2 - (display_lines / 2) * fontSize;

        rl::Vector2 input_pos = {static_cast<float>(X0), static_cast<float>(Y0)};
        float scaleFactor = fontSize/font.baseSize;

        auto line_range = lb.range(first_line, first_line + display_lines);
        auto next_line_iter = line_range.begin();
        auto last_line_iter = line_range.end();
        std::cout << next_line_iter->first_codepoint().pos() << ' ';
        auto codepoint_iter = (next_line_iter++)->first_codepoint().resolve();
        // auto attribute_iter = lb.get_attribute_iter(codepoint_iter);
        bool end_reached = false;

        while (!end_reached) {
            int codepoint = *codepoint_iter;
            rl::DrawTextCodepoint(font, codepoint, input_pos, fontSize, rl::BLACK);
            input_pos.x += ((float)font.chars[codepoint].advanceX*scaleFactor);

            codepoint_iter++;
            // attribute_iter++;
            while (codepoint_iter == next_line_iter->first_codepoint() && !end_reached) {
                if (next_line_iter == last_line_iter) {
                    end_reached = true;
                } else {
                    std::cout << next_line_iter->first_codepoint().pos() << ' ';
                    next_line_iter++;
                    input_pos.x = X0;
                    input_pos.y += fontSize;
                }
            }
        }
        std::cout << next_line_iter->first_codepoint().pos() << '\n';

        rl::DrawRectangle(X0 - 4, Y0, 2, input_pos.y - Y0 + fontSize, rl::LIGHTGRAY);
        rl::DrawRectangle(X0 + 2 + font.chars['X'].advanceX*scaleFactor*line_width, Y0, 2, input_pos.y - Y0 + fontSize, rl::LIGHTGRAY);

        rl::EndDrawing();
    }

    rl::CloseWindow();

    return 0;
}
#endif
