#include "SelectionProvider.hpp"

terminal::SelectionProvider::SelectionProvider(Selection& selection, const TextRenderer& renderer):
    selection(selection), renderer(renderer) { }

bool terminal::SelectionProvider::mouse_movement_threshold(rl::Vector2 pos_a, rl::Vector2 pos_b) {
    rl::Vector2 diff = {
        pos_a.x - pos_b.x,
        pos_a.y - pos_b.y
    };

    return diff.x*diff.x + diff.y*diff.y > 1;  // arbitrary relatively low value
}

void terminal::SelectionProvider::update() {
    if (rl::IsMouseButtonPressed(rl::MOUSE_BUTTON_LEFT)) {
        if (rl::IsKeyDown(rl::KEY_LEFT_SHIFT)) {
            // shift-click, update second selection pos
            renderer.get_hovered_char(rl::GetMousePosition(), selection.line_b, selection.col_b);
        } else {
            // mouse button is pressed down, set first selection position and unset second one
            mouse_click_start_pos = rl::GetMousePosition();
            renderer.get_hovered_char(mouse_click_start_pos, selection.line_a, selection.col_a);
            selection.line_b = nullptr;
        }
    }

    if (rl::IsMouseButtonDown(rl::MOUSE_BUTTON_LEFT)) {
        rl::Vector2 mouse_pos = rl::GetMousePosition();
        if (selecting || mouse_movement_threshold(mouse_pos, mouse_click_start_pos)) {
            // mouse button is held down, constantly update second position after threshold was reached
            selecting = true;
            renderer.get_hovered_char(mouse_pos, selection.line_b, selection.col_b);
        }
    }

    if (rl::IsMouseButtonReleased(rl::MOUSE_BUTTON_LEFT)) {
        selecting = false;
    }
}
