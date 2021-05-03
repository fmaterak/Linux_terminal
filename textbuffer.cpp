#include <utility>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "textbuffer.h"

TextBuffer::TextBuffer() {
  Reset();
}

TextBuffer::TextBuffer(uint32_t w, uint32_t h) {
  Reset();
  Resize(w, h);
}

void TextBuffer::ChangeAttribute(CharAttrib attribute) {
  current_attribute_ = attribute;
}

void TextBuffer::ChangeForegroundColor(uint8_t r, uint8_t g, uint8_t b) {
  current_attribute_.fg.r = r;
  current_attribute_.fg.g = g;
  current_attribute_.fg.b = b;
}

void TextBuffer::ChangeBackgroundColor(uint8_t r, uint8_t g, uint8_t b) {
  current_attribute_.bg.r = r;
  current_attribute_.bg.g = g;
  current_attribute_.bg.b = b;
}

void TextBuffer::Clear() {
  std::fill(characters_.begin(), characters_.end(), 0);
}

void TextBuffer::Reset() {
  current_attribute_ = kDefaultAttribute;
  w_ = 0;
  h_ = 0;
  cursor_x_ = 0;
  cursor_y_ = 0;
  scroll_y_ = 0;
  Clear();
}

