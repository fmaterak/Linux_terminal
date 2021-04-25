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

	// rl::SetConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
    rl::InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    // NOTE: extures and fonts must be loaded after window initialization (OpenGL context is required)

    rl::Font font = rl::LoadFontEx("resources/Space_Mono/SpaceMono-Regular.ttf", 64, 0, 0);

    float fontSize = font.baseSize / 2;
    rl::Vector2 fontPosition = { 40, (screenHeight - fontSize) / 2 };
    // rl::Vector2 textSize = { 0.0f, 0.0f };

    // Setup texture scaling filter
    SetTextureFilter(font.texture, rl::TEXTURE_FILTER_BILINEAR);

    rl::SetTargetFPS(60);

    while (!rl::WindowShouldClose()) {
        rl::BeginDrawing();
        rl::ClearBackground(rl::RAYWHITE);
        rl::DrawTextEx(font, "Test", fontPosition, fontSize, 0, rl::BLACK);
        rl::EndDrawing();
    }

    rl::CloseWindow();

    return 0;
}
#endif
