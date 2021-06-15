#pragma once

#include "Reader.hpp"

#include <iostream>

namespace terminal {

class StreamReader: public Reader {
    bool prev_space;
    std::istream& stream;

public:
    StreamReader(std::istream& stream);

    int next_codepoint(bool& style_changed, Style& new_style);
};

};
