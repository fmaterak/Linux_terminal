#pragma once

#include "Selection.hpp"
#include "TextRenderer.hpp"

namespace rl {
    extern "C" {
        #include "raylib.h"
    }
}

namespace terminal {

class SelectionProvider {
    Selection& selection;
    const TextRenderer& renderer;

    bool selecting;
    rl::Vector2 mouse_click_start_pos;

    static bool mouse_movement_threshold(rl::Vector2 pos_a, rl::Vector2 pos_b);

public:
    SelectionProvider(Selection& selection, const TextRenderer& renderer);

    void update();
};

};
