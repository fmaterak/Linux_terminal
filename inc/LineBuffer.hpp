#pragma once

#include <list>
#include <climits>
#include <iostream>
#include <algorithm>

namespace rl {
	extern "C" {
		#include "raylib.h"
	}
}

#include "Snake.hpp"

namespace terminal {

class LineBuffer {
public:
    using codepoint=int;

    enum FontProp: unsigned char {
        BOLD         = 1 << 0,
        DIM          = 1 << 1,
        ITALIC       = 1 << 2,
        UNDERLINED   = 1 << 3,
        BLINK        = 1 << 4,
        INVERTED     = 1 << 5,
        HIDDEN       = 1 << 6,
    };

    struct CharAttr {
        unsigned char bg, fg;
        FontProp font;
    };

    class Line {
        Snake<codepoint>::Iterator first;
        bool hard_wrapped;

        friend class LineBuffer;

    public:
        inline Line(Snake<codepoint>::Iterator first_codepoint, bool is_hard_wrapped):
            first(first_codepoint), hard_wrapped(is_hard_wrapped) { }
        inline bool is_hard_wrapped() const { return hard_wrapped; }
        inline Snake<codepoint>::Iterator first_codepoint() const { return first; }
    };

    using LineList = std::list<Line>;
    using LineListIter = LineList::iterator;

    class LineRange {
        // std::list has no quick random access, this class implements some logic to move around as quickly as possible
        // it is based on two facts:
        //   - iterators can be moved by a single step only, so moving forward by 5 requires 5 increments
        //   - LineRange is used to iterate over lines which are currently displayed, therefore
        //     as long as a view is not scrolled the range remains the same
        // so in order to decrease the number of increment/decrement operations on iterator,
        // previously requested range is cached (as it's unlikely to change drastically)
        // and start-point-choosing alghorithm is implemented (see move())
        LineBuffer& parent;
        std::size_t cached_begin_pos, cached_end_pos;
        LineListIter cached_begin, cached_end;

        void reset_cache();
        void new_lines_inserted(std::size_t after_pos);
        void move(std::size_t begin_pos, std::size_t end_pos);

        friend class LineBuffer;

    public:
        LineRange(LineBuffer& parent);
        inline LineListIter begin() const { return cached_begin; }
        inline LineListIter end() const { return cached_end; }
    };

private:
    int width;
    // Snake<Attr> attributes;
    // Snake<Attr>::Iterator attributes_iter;
    Snake<codepoint> codepoints;
    // Snake<int>::Iterator codepoints_iter;
    LineList lines;
    LineRange line_range;
    std::size_t max_lines, first_line;

public:
    LineBuffer(std::size_t max_lines, int width);

    inline std::size_t first_line_num() const { return first_line; }
    inline std::size_t last_line_num() const { return first_line + lines.size() - 1; }

    const LineRange& range(std::size_t begin_pos, std::size_t end_pos);

    void read_from(std::istream& stream, std::size_t count = -1);
    void set_line_width(int new_width);
};

};
