#include "LineBuffer.hpp"

terminal::LineBuffer::LineRange::LineRange(LineBuffer& parent): parent(parent) {
    reset_cache();
}

void terminal::LineBuffer::LineRange::reset_cache() {
    cached_begin_pos = 0;
    cached_end_pos = parent.lines.size() - 1;
    cached_begin = parent.lines.begin();
    cached_end = std::prev(parent.lines.end());
}

void terminal::LineBuffer::LineRange::new_lines_inserted(std::size_t after_pos) {
    if (cached_end_pos > after_pos) {
        cached_end = std::prev(parent.lines.end());
        cached_end_pos = parent.lines.size() - 1;
    }
    if (cached_begin_pos > after_pos) {
        // begin is always before end, so if begin is after pos, end is after pos too
        // tl;dr: this will use values from if block above
        cached_begin = cached_end;
        cached_begin_pos = cached_end_pos;
    }
}

void terminal::LineBuffer::LineRange::move(std::size_t begin_pos, std::size_t end_pos) {
    // each iterator's new position can be achieved by moving from one of three start points:
    //   - cached_begin
    //   - cached_end
    //   - another new iterator
    // this algorithm chooses the best option in order to decrease
    // the number of iterator increment/decrement operations needed

    // there is always a dummy line at the end of parent.lines, can't request that
    // (it's like end iterator)
    if (end_pos >= parent.lines.size() || end_pos < begin_pos) {
        throw std::invalid_argument("end_pos");
    }

    // end_pos++;

    struct {
        std::size_t cost;
        LineListIter iter;
        bool incr;

        void move() {
            if (incr) {
                while (cost--) iter++;
            } else {
                while (cost--) iter--;
            }
        }
    } paths[2];
    std::size_t positions[2] = {begin_pos, end_pos};

    // step 1: get shortest path to new begin & new end from cached iterators
    for (int i = 0; i < 2; i++) {
        std::size_t pos = positions[i];
        if (pos >= cached_end_pos) {
            // required pos is after cached range's end, shortest path: cached_end++
            paths[i] = { pos - cached_end_pos, cached_end, true };
        }
        else if (pos <= cached_begin_pos) {
            // required pos is before cached range's start, shortest path: cached_begin--
            paths[i] = { cached_begin_pos - pos, cached_begin, false };
        }
        else {
            // required pos is inside cached range, determine best start based on path length (cost)
            std::size_t cost_from_begin = pos - cached_begin_pos;
            std::size_t cost_from_end = cached_end_pos - pos;
            if (cost_from_begin > cost_from_end) {
                // the pos is closer to cached_end, cached_end--
                paths[i] = { cost_from_end, cached_end, false };
            } else {
                // the pos is closed to cached_begin, cached_begin++
                paths[i] = { cost_from_begin, cached_begin, true };
            }
        }
    }

    // step 2: choose less expensive operation, move corresponding iterator
    int moved = paths[0].cost > paths[1].cost;
    paths[moved].move();

    // step 3: an iterator not moved yet now has an option to choose moved one as a start point ("relative" move)
    std::size_t rel_cost = end_pos - begin_pos;  // the size of new range, i.e. the distance between its start & end
    if (rel_cost < paths[!moved].cost) {
        // if moved == 0, we're going to move from new start, so increment (0 -> true, 1 -> false)
        paths[!moved] = { rel_cost, paths[moved].iter, !moved };
    }

    // step 4: move unmoved iterator
    paths[!moved].move();

    // last step: update cached iterators & positions
    cached_begin_pos = begin_pos;
    cached_end_pos = end_pos;
    cached_begin = paths[0].iter;
    cached_end = paths[1].iter;
}


terminal::LineBuffer::LineBuffer(int width):
    lines({{codepoints.end(), true}, {codepoints.end(), true}}), line_range(*this), width(width) { }

const terminal::LineBuffer::LineRange& terminal::LineBuffer::range(std::size_t begin_pos, std::size_t end_pos) {
    line_range.move(begin_pos, end_pos);
    return line_range;
}

void terminal::LineBuffer::read_from(std::istream& stream, std::size_t count) {
    // TODO: advance head by count?
    char c;
    int width_counter;
    std::size_t last_line_pos_before_insertion = lines.size() - 2;
    auto dummy_line = std::prev(lines.end());
    // if (lines.size() <= 1) {  // are there any real lines? (not dummy one)
        // width_counter = 0;
    // } else {
        width_counter = codepoints.head_pos() - std::prev(dummy_line)->first_codepoint().pos();
    // }

    while (count--) {
        c = stream.get();
        if (c == '\n') {
            width_counter = 0;
            lines.emplace(dummy_line, codepoints.end(), true);
        } else {
            codepoints.push(c);
            if (++width_counter == width) {
                width_counter = 0;
                lines.emplace(dummy_line, codepoints.end(), false);
            }
        }
    }

    dummy_line->first = codepoints.end();
    line_range.new_lines_inserted(last_line_pos_before_insertion);
}
