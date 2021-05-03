#include <iostream>

namespace rl {
	extern "C" {
		#include "raylib.h"
	}
}

#include "Snake.hpp"

#ifndef SNAKE_BLOCKS_TEST_MAIN
// int main() {
//     terminal::Snake<char> snake;
//     snake.read_from_string("hello, world!");
//     std::cout << "Snake spans from " << snake.tail_pos() << " to " << snake.head_pos() << '\n';
//     std::cout << "Snake contents: ";
//     for (char c: snake) {
//         std::cout << c;
//     }
//     std::cout << '\n';
//     std::cout << "Advancing by 7\n";
//     snake.advance_tail(7);
//     std::cout << "Snake spans from " << snake.tail_pos() << " to " << snake.head_pos() << '\n';
//     std::cout << "Snake contents: ";
//     for (char c: snake) {
//         std::cout << c;
//     }
//     std::cout << '\n';
// }

int main() {
    const int screenWidth = 640;
    const int screenHeight = 480;

    const char msg1[] = "hello world!";
    const char msg2[] = "witaj świat!";
    const char msg3[] = "привет мир!";

    terminal::Snake<int> snake;

	// rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // NOTE: extures and fonts must be loaded after window initialization (OpenGL context is required)
    rl::Font font = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-regular.ttf", 64, 0, 2048);

    // Setup texture scaling filter
    SetTextureFilter(font.texture, rl::TEXTURE_FILTER_BILINEAR);

    float fontSize = font.baseSize / 2;

    rl::SetTargetFPS(60);

    while (!rl::WindowShouldClose()) {
        int chr;
        while ((chr = rl::GetCharPressed())) snake.push(chr);
        if (rl::IsKeyPressed(rl::KEY_BACKSPACE)) snake.advance_tail(snake.size());
        rl::BeginDrawing();
        rl::ClearBackground(rl::RAYWHITE);
        rl::DrawTextEx(font, msg1, { 40, (screenHeight - fontSize) / 2 - fontSize }, fontSize, 0, rl::BLACK);
        rl::DrawTextEx(font, msg2, { 40, (screenHeight - fontSize) / 2 }, fontSize, 0, rl::BLACK);
        rl::DrawTextEx(font, msg3, { 40, (screenHeight - fontSize) / 2 + fontSize}, fontSize, 0, rl::BLACK);

        rl::Vector2 input_pos = {40, (screenHeight - fontSize) / 2 + 2 * fontSize};
        float scaleFactor = fontSize/font.baseSize;
        for (int codepoint: snake) {
            rl::DrawTextCodepoint(font, codepoint, input_pos, fontSize, rl::BLACK);
            input_pos.x += ((float)font.chars[codepoint].advanceX*scaleFactor);
        }
        rl::EndDrawing();
    }

    rl::CloseWindow();

    return 0;
}
#endif
