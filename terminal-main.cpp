#include <iostream>

namespace rl {
	extern "C" {
		#include "raylib.h"
	}
}

#include "SnakeBlocks.hpp"

#ifndef SNAKE_BLOCKS_TEST_MAIN
int main() {
    const int screenWidth = 640;
    const int screenHeight = 480;

    const char msg1[] = "hello world!";
    const char msg2[] = "witaj świat!";
    const char msg3[] = "привет мир!";

	// rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // NOTE: extures and fonts must be loaded after window initialization (OpenGL context is required)
    rl::Font font = rl::LoadFontEx("resources/Roboto_Mono/static/RobotoMono-regular.ttf", 64, 0, 2048);

    // Setup texture scaling filter
    SetTextureFilter(font.texture, rl::TEXTURE_FILTER_BILINEAR);

    float fontSize = font.baseSize / 2;

    rl::SetTargetFPS(60);

    while (!rl::WindowShouldClose()) {
        // wchar_t chr;
        // while ((chr = rl::GetCharPressed())) std::cout << chr << "\n";
        rl::BeginDrawing();
        rl::ClearBackground(rl::RAYWHITE);
        rl::DrawTextEx(font, msg1, { 40, (screenHeight - fontSize) / 2 - fontSize }, fontSize, 0, rl::BLACK);
        rl::DrawTextEx(font, msg2, { 40, (screenHeight - fontSize) / 2 }, fontSize, 0, rl::BLACK);
        rl::DrawTextEx(font, msg3, { 40, (screenHeight - fontSize) / 2 + fontSize}, fontSize, 0, rl::BLACK);
        rl::EndDrawing();
    }

    rl::CloseWindow();

    return 0;
}
#endif
