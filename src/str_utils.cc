#include "str_utils.h"

#include <string>
#include "unicode_script.h"
#include "util.h"

namespace sentencepiece {
namespace str_utils {

bool is_digit(char32 c) {
  return c >= '0' && c <= '9';
}

bool is_alpha(char32 c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool is_mask(char32 c) {
  // 0x2582 is '▂', used as mask for whitespace during training.
  return c == 0x2582;
}

bool is_dash(char32 c) {
  return c == '-';
}

bool is_pure_digits(const string_util::UnicodeText &piece) {
  for (char32 c : piece) {
    if (!is_digit(c)) return false;
  }
  return true;
}

size_t monospace_size(const string_util::UnicodeText &sentencepiece) {
  size_t size = 0;
  for (const char32 c : sentencepiece) {
    auto script = unicode_script::GetScript(c);
    // Treat CJK characters (Han, Hiragana, Katakana, Hangul) as size 2
    if (script == unicode_script::U_Han ||
        script == unicode_script::U_Hiragana ||
        script == unicode_script::U_Katakana ||
        script == unicode_script::U_Hangul ||
        c == 0x30FC) {
      size += 2;
    } else {
      size += 1;
    }
  }
  return size;
}

bool is_beg_or_end_with_one_char(const string_util::UnicodeText &piece) {
  if (piece.size() < 3) {
    return false;
  }

  for (char32 c : piece) {
    if (!is_alpha(c) && !is_digit(c) && !is_mask(c) && !is_dash(c)) {
      return false;
    }
  }

  if ((is_alpha(piece[0]) && is_mask(piece[1])) || 
      (is_mask(piece[piece.size() - 2]) && is_alpha(piece[piece.size() - 1]))) {
    return true;
  }

  return false;
}


bool is_malformed(const string_util::UnicodeText &piece) {
  std::string piece_str = string_util::UnicodeTextToUTF8(piece);
  
  // check for special chars
  if (piece_str.size() >= 2 &&
      piece_str.find('%') != std::string::npos) {
    return true;
  }

  // check for multiple consecutive dots or dashes
  size_t consecutive_dots = 0;
  for (char c : piece_str) {
    if (c == '.' || c == '-') {
      consecutive_dots++;
      if (consecutive_dots >= 2) {
        return true;
      }
    } else {
      consecutive_dots = 0;
    }
  }

  // check for stripped (start/end with) chars
  // 0x2582 is '▂', used as mask for whitespace during train
  if (piece_str.size() >= 2 &&
      (piece_str.front() == '.' || piece_str.back() == '.' ||
       piece_str.front() == '-' || piece_str.back() == '-' ||
       is_mask(piece[0]) || is_mask(piece[piece.size() - 1]))) {
    return true;
  }

  // check for bv numbers (start with 'bv[\d]')
  if (piece_str.size() >= 3 && piece_str.substr(0, 2) == "bv" &&
      is_digit(piece_str[2])) {
    return true;
  }

  return false;
}

}  // namespace str_utils
}  // namespace sentencepiece