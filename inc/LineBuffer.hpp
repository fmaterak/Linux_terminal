#pragma once

#include "Snake.hpp"
#include "Reader.hpp"

namespace rl {
	extern "C" {
		#include "raylib.h"
	}
}

#include <list>
#include <climits>
#include <iostream>
#include <algorithm>

namespace terminal {

class LineBuffer {
public:
    struct StyleChange {
        Style new_style;
        std::size_t effective_from;
    };

    class Line {
        bool hard_wrapped;
        Snake<codepoint>::Iterator first;
        std::list<StyleChange>::iterator style;

        friend class LineBuffer;

    public:
        inline Line(
            bool is_hard_wrapped,
            Snake<codepoint>::Iterator first_codepoint,
            std::list<StyleChange>::iterator style):
            hard_wrapped(is_hard_wrapped), first(first_codepoint), style(style) { }
        inline bool operator==(const Line& other) { return first == other.first; }
        inline bool is_hard_wrapped() const { return hard_wrapped; }
        inline Snake<codepoint>::Iterator first_codepoint() const { return first; }
        inline std::list<StyleChange>::iterator active_style() const { return style; }
    };

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
        std::list<Line>::iterator cached_begin, cached_end;

        void reset_cache();
        void new_lines_inserted(std::size_t after_pos);
        void move(std::size_t begin_pos, std::size_t end_pos);

        friend class LineBuffer;

    public:
        LineRange(LineBuffer& parent);
        inline std::list<Line>::iterator begin() const { return cached_begin; }
        inline std::list<Line>::iterator end() const { return cached_end; }
    };

private:
    int width;
    std::list<StyleChange> style_changes;
    Snake<codepoint> codepoints;
    std::list<Line> lines;
    LineRange line_range;
    std::size_t max_lines, first_line;

public:
    LineBuffer(std::size_t max_lines, int width);

    inline std::size_t first_line_num() const { return first_line; }
    inline std::size_t last_line_num() const { return first_line + lines.size() - 1; }

    const LineRange& range(std::size_t begin_pos, std::size_t end_pos);

    void read_from(Reader& reader, std::size_t count = -1);
    void set_line_width(int new_width);
};

};
