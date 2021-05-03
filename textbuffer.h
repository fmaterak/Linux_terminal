#pragma once
#include <cstdint>
#include <utility>
#include <vector>

struct Color {
  uint8_t r, g, b;
};

struct CharAttrib {
  Color fg;  // Foreground
  Color bg;  // Background
};

class TextBuffer {
 private:
  std::vector<uint32_t> characters_;
  std::vector<CharAttrib> attributes_;
  uint32_t w_;  // In characters.
  uint32_t h_;  // In characters.

  const CharAttrib kDefaultAttribute = {
      { 192, 192, 192 },
      { 0, 0, 0 },
  };

  CharAttrib current_attribute_;
  uint32_t cursor_x_;
  uint32_t cursor_y_;

 public:
  TextBuffer();
  TextBuffer(uint32_t w, uint32_t h);
  void ChangeAttribute(CharAttrib attribute);
  void ChangeForegroundColor(uint8_t r, uint8_t g, uint8_t b);
  void ChangeBackgroundColor(uint8_t r, uint8_t g, uint8_t b);
  void Clear();
  void Reset();
};