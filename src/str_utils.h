#ifndef STR_UTILS_H_
#define STR_UTILS_H_

#include "common.h"
#include "util.h"

namespace sentencepiece {
namespace str_utils {

// Character classification functions
bool is_digit(char32 c);
bool is_alpha(char32 c);
bool is_mask(char32 c);
bool is_dash(char32 c);

// String analysis functions
bool is_pure_digits(const string_util::UnicodeText &piece);
size_t monospace_size(const string_util::UnicodeText &sentencepiece);
bool is_beg_or_end_with_one_char(const string_util::UnicodeText &piece);
bool is_malformed(const string_util::UnicodeText &piece);

}  // namespace str_utils
}  // namespace sentencepiece

#endif  // STR_UTILS_H_