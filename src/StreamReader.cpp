#include "StreamReader.hpp"

terminal::StreamReader::StreamReader(std::istream& stream): stream(stream), prev_space(true) { }

terminal::codepoint terminal::StreamReader::next_codepoint(bool& style_changed, terminal::Style& new_style) {
    if (stream.eof()) {
        return EOF;
    }

    codepoint c = stream.get();

    if (c == ' ') {
        prev_space = true;
        style_changed = true;
        new_style = {Style::NONE};
    }
    else if (prev_space) {
        prev_space = false;
        style_changed = true;

        switch (c) {
            case 'i': new_style = {Style::ITALIC}; break;
            case 'b': new_style = {Style::BOLD}; break;
            // case 'd': new_style = {Style::DIM}; break;
            case 'u': new_style = {Style::UNDERLINED}; break;
            case 'v': new_style = {Style::INVERTED}; break;
            // case 'k': new_style = {Style::BLINK}; break;
            case 's': new_style = {Style::STRIKEOUT}; break;
            default: style_changed = false;
        }
    }

    return c;
}
