#pragma once

namespace terminal {

struct Style {
    enum StyleMask: unsigned short {
        NONE         = 0,
        BOLD         = 1 << 0,
        DIM          = 1 << 1,
        ITALIC       = 1 << 2,
        UNDERLINED   = 1 << 3,
        BLINK        = 1 << 4,
        INVERTED     = 1 << 5,
        HIDDEN       = 1 << 6,
        STRIKEOUT    = 1 << 7,
        FG_COLOR     = 1 << 8,
        BG_COLOR     = 1 << 9,
    };

    StyleMask mask;
    unsigned char bg, fg;  // up to 256 color palette, these members contain color number
};

};
