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
    // check if cached iterators are invalidated
    if (cached_end_pos >= after_pos) {
        cached_end = std::prev(parent.lines.end());
        cached_end_pos = parent.lines.size() - 1;
    }
    if (cached_begin_pos >= after_pos) {
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

    struct {
        std::size_t cost;
        std::list<Line>::iterator iter;
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


terminal::LineBuffer::LineBuffer(std::size_t max_lines, int width):
    first_line(0), max_lines(max_lines), width(width), line_range(*this)
{
    // both lists contain a dummy object at the end used to safeley dereference iterator
    style_changes = std::list<StyleChange>(2, {Style::NONE});
    // codepoints.begin() and codepoints.end() are the same at this point
    lines = std::list<Line>(2, {true, codepoints.begin(), style_changes.begin()});
}

const terminal::LineBuffer::LineRange& terminal::LineBuffer::range(std::size_t begin_pos, std::size_t end_pos) {
    line_range.move(begin_pos, end_pos);
    return line_range;
}

void terminal::LineBuffer::read_from(Reader& reader, std::size_t count) {
    // TODO: advance head by count?
    // TODO: implement advancing tail after exceeding max_lines
    codepoint c;
    bool style_changed = false;
    Style style;  // maybe shold be initialized with current style?

    auto dummy_line_iter = std::prev(lines.end());
    auto dummy_style_change_iter = std::prev(style_changes.end());
    auto active_style = std::prev(dummy_style_change_iter);

    std::size_t last_line_pos_before_insertion = lines.size() - 2;
    int width_counter = codepoints.head_pos() - std::prev(dummy_line_iter)->first_codepoint().pos();

    while (count--) {
        c = reader.next_codepoint(style_changed, style);
        if (c == EOF) {
            break;
        }
        if (style_changed) {
            active_style = style_changes.insert(dummy_style_change_iter, {style, codepoints.head_pos()});
            style_changed = false;
        }
        if (c == '\n') {
            width_counter = 0;
            lines.emplace(dummy_line_iter, true, codepoints.end(), active_style);
        }
        else {
            if (width_counter == width) {
                width_counter = 0;
                lines.emplace(dummy_line_iter, false, codepoints.end(), active_style);
            }
            codepoints.push(c);
            width_counter++;
        }
    }

    dummy_line_iter->first = codepoints.end();
    dummy_style_change_iter->effective_from = codepoints.head_pos();  // wtf
    line_range.new_lines_inserted(last_line_pos_before_insertion);
}

void terminal::LineBuffer::set_line_width(int new_width) {
    width = new_width;

    auto phys_line = lines.begin();  // the first line is always physical (i.e. hard-wrapped)
    auto end = std::prev(lines.end());  // don't erase dummy line

    while (phys_line != end) {
        // find the next physical line and erase soft lines in between
        auto after_phys_line = std::next(phys_line);
        auto next_phys_line = std::find_if(after_phys_line, end, [](Line& l){ return l.is_hard_wrapped(); });
        lines.erase(after_phys_line, next_phys_line);

        // styles
        auto active_style = phys_line->active_style();
        auto next_style = std::next(active_style);

        // add new soft lines
        std::size_t first_pos = phys_line->first.pos();
        std::size_t line_width = next_phys_line->first.pos() - first_pos;
        std::size_t soft_wrap_col = 0;
        while (line_width - soft_wrap_col > width) {
            soft_wrap_col += width;
            std::size_t new_line_beginning = first_pos + soft_wrap_col;
            while (next_style->effective_from <= new_line_beginning) {
                active_style = next_style++;
            }
            lines.emplace(next_phys_line, false, codepoints.iter(new_line_beginning), active_style);
        }

        // move iterator
        phys_line = next_phys_line;
    }

    line_range.reset_cache();
}
