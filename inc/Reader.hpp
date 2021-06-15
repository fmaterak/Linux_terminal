#pragma once

#include "Style.hpp"

namespace terminal {

using codepoint=int;

class Reader {
public:
    virtual codepoint next_codepoint(bool& style_changed, Style& new_style) = 0;
};

};
