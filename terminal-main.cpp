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
    const int screenWidth = 640;
    const int screenHeight = 480;

    const char msg1[] = "hello world!";
    const char msg2[] = "witaj świat!";
    const char msg3[] = "привет мир!";

    terminal::LineBuffer lb(16);
    std::ifstream file;
    file.open("display_data.txt");
    if (!file.is_open()) {
        std::cout << "Error opening file" << std::endl;
        return 1;
    }
    lb.read_from(file, 34);
    file.close();

	// rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // NOTE: extures and fonts must be loaded after window initialization (OpenGL context is required)
    rl::Font font = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-regular.ttf", 64, 0, 2048);

    // Setup texture scaling filter
    SetTextureFilter(font.texture, rl::TEXTURE_FILTER_BILINEAR);

    float fontSize = font.baseSize / 2;

    rl::SetTargetFPS(60);

    while (!rl::WindowShouldClose()) {
        // int chr;
        // while ((chr = rl::GetCharPressed())) lb.push(chr);
        // if (rl::IsKeyPressed(rl::KEY_BACKSPACE)) snake.advance_tail(snake.size());
        rl::BeginDrawing();
        rl::ClearBackground(rl::RAYWHITE);
        // rl::DrawTextEx(font, msg1, { 40, (screenHeight - fontSize) / 2 - fontSize }, fontSize, 0, rl::BLACK);
        // rl::DrawTextEx(font, msg2, { 40, (screenHeight - fontSize) / 2 }, fontSize, 0, rl::BLACK);
        // rl::DrawTextEx(font, msg3, { 40, (screenHeight - fontSize) / 2 + fontSize}, fontSize, 0, rl::BLACK);

        rl::Vector2 input_pos = {40, (screenHeight - fontSize) / 2 - fontSize};
        float scaleFactor = fontSize/font.baseSize;

        auto line_range = lb.range(0, 4);
        auto next_line_iter = line_range.begin();
        auto last_line_iter = line_range.end();
        std::cout << next_line_iter->first_codepoint().pos() << ' ';
        auto codepoint_iter = (next_line_iter++)->first_codepoint().resolve();
        // auto attribute_iter = lb.get_attribute_iter(codepoint_iter);

        while (true) {
            int codepoint = *codepoint_iter;
            rl::DrawTextCodepoint(font, codepoint, input_pos, fontSize, rl::BLACK);
            input_pos.x += ((float)font.chars[codepoint].advanceX*scaleFactor);

            codepoint_iter++;
            // attribute_iter++;
            if (codepoint_iter == next_line_iter->first_codepoint()) {
                if (next_line_iter->first_codepoint() == last_line_iter->first_codepoint()) {
                    break;
                }
                std::cout << next_line_iter->first_codepoint().pos() << ' ';
                next_line_iter++;
                input_pos.x = 40;
                input_pos.y += fontSize;
            }
        }
        std::cout << next_line_iter->first_codepoint().pos() << '\n';
        rl::EndDrawing();
    }

    rl::CloseWindow();

    return 0;
}
#endif
