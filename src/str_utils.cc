#include "str_utils.h"

#include <algorithm>
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

bool is_alnum(char32 c) {
  return is_alpha(c) || is_digit(c);
}

bool is_mask(char32 c) {
  // 0x2582 is '▂', used as mask for whitespace during training.
  return c == 0x2582;
}

bool is_dash(char32 c) {
  return c == '-' || c == '=' || c == '_'; 
}

bool is_connector(char32 c) {
  return c == '.' || is_dash(c);
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
    if (!is_alnum(c) && !is_mask(c) && !is_dash(c)) {
      return false;
    }
  }

  if ((is_alnum(piece[0]) && is_mask(piece[1])) || 
      (is_mask(piece[piece.size() - 2]) && is_alnum(piece[piece.size() - 1]))) {
    return true;
  }

  return false;
}


namespace {
bool is_ascii_piece(const string_util::UnicodeText &piece) {
  for (const char32 c : piece) {
    if (c > 0x7f) {
      return false;
    }
  }
  return true;
}

bool has_repeated_ascii_run(const std::string &piece_str, size_t min_run = 3) {
  if (piece_str.size() < min_run) {
    return false;
  }
  size_t run_len = 1;
  for (size_t i = 1; i < piece_str.size(); ++i) {
    if (piece_str[i] == piece_str[i - 1]) {
      ++run_len;
      if (run_len >= min_run) {
        return true;
      }
    } else {
      run_len = 1;
    }
  }
  return false;
}

bool contains_utf8(const std::string &piece_str, const char *needle) {
  return piece_str.find(needle) != std::string::npos;
}

std::string ascii_lower(std::string piece_str) {
  std::transform(piece_str.begin(), piece_str.end(), piece_str.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return piece_str;
}

bool is_video_id_piece(const std::string &piece_str) {
  const std::string lower = ascii_lower(piece_str);
  if (lower.size() >= 6 && lower.rfind("av", 0) == 0) {
    return std::all_of(lower.begin() + 2, lower.end(),
                       [](unsigned char c) { return c >= '0' && c <= '9'; });
  }
  if (lower.size() >= 6 && lower.rfind("bv", 0) == 0) {
    return std::all_of(lower.begin() + 2, lower.end(), [](unsigned char c) {
      return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z');
    });
  }
  return false;
}

bool is_curated_phrase_noise(const std::string &piece_str) {
  static constexpr const char *kExactNoises[] = {
      "一键三连", "求三连", "求关注", "快来围观", "账号已注销",
      "外部链接", "外部连结", "参考来源", "扩展阅读", "存档副本",
      "移动重定向", "简繁重定向", "别名重定向"};

  for (const char *noise : kExactNoises) {
    if (piece_str == noise) {
      return true;
    }
  }

  if (contains_utf8(piece_str, "直播间") && piece_str != "直播间") {
    return true;
  }
  if (contains_utf8(piece_str, "如侵删") || contains_utf8(piece_str, "侵删") ||
      contains_utf8(piece_str, "素材来源") || contains_utf8(piece_str, "未经允许") ||
      contains_utf8(piece_str, "禁止搬运") || contains_utf8(piece_str, "禁止转载") ||
      contains_utf8(piece_str, "非商用")) {
    return true;
  }
  if (contains_utf8(piece_str, "仅搬运") || contains_utf8(piece_str, "搬运自") ||
      contains_utf8(piece_str, "转载自") || contains_utf8(piece_str, "投稿请") ||
      contains_utf8(piece_str, "投稿至") || contains_utf8(piece_str, "投稿见") ||
      contains_utf8(piece_str, "原搬运") || contains_utf8(piece_str, "原投稿") ||
      contains_utf8(piece_str, "接投稿") || contains_utf8(piece_str, "代投稿") ||
      contains_utf8(piece_str, "勿转载")) {
    return true;
  }

  static constexpr const char *kActionWords[] = {
      "点赞", "投币", "收藏", "转发", "关注", "三连", "私信"};
  int action_hits = 0;
  for (const char *action : kActionWords) {
    if (contains_utf8(piece_str, action)) {
      ++action_hits;
    }
  }
  if (action_hits >= 2) {
    return true;
  }
  if (action_hits >= 1 &&
      (contains_utf8(piece_str, "请") || contains_utf8(piece_str, "记得") ||
       contains_utf8(piece_str, "欢迎") || contains_utf8(piece_str, "感谢") ||
       contains_utf8(piece_str, "给个") || contains_utf8(piece_str, "点个") ||
       contains_utf8(piece_str, "支持") || contains_utf8(piece_str, "一下") ||
       contains_utf8(piece_str, "主页") || contains_utf8(piece_str, "首页") ||
       contains_utf8(piece_str, "简介") || contains_utf8(piece_str, "评论区") ||
       contains_utf8(piece_str, "置顶") || contains_utf8(piece_str, "传送门") ||
       contains_utf8(piece_str, "惊喜") || contains_utf8(piece_str, "链接"))) {
    return true;
  }

  return false;
}
}  // namespace


bool is_malformed(const string_util::UnicodeText &piece) {
  std::string piece_str = string_util::UnicodeTextToUTF8(piece);

  if (is_video_id_piece(piece_str) || is_curated_phrase_noise(piece_str)) {
    return true;
  }
  
  // check for special chars
  if (piece_str.size() >= 2 &&
      piece_str.find('%') != std::string::npos) {
    return true;
  }

  // check for multiple consecutive dots or dashes
  size_t consecutive_connectors = 0;
  size_t connector_count = 0;
  for (char c : piece_str) {
    if (c == '.' || c == '-' || c == '_' || c == '=') {
      ++connector_count;
      ++consecutive_connectors;
      if (consecutive_connectors >= 2) {
        return true;
      }
    } else {
      consecutive_connectors = 0;
    }
  }

  // check for stripped (start/end with) chars
  // 0x2582 is '▂', used as mask for whitespace during train
  if (piece_str.size() >= 2 &&
      (piece_str.front() == '.' || piece_str.back() == '.' ||
       piece_str.front() == '-' || piece_str.back() == '-' ||
       piece_str.front() == '_' || piece_str.back() == '_' ||
       piece_str.front() == '=' || piece_str.back() == '=' ||
       is_mask(piece[0]) || is_mask(piece[piece.size() - 1]))) {
    return true;
  }

  if (is_ascii_piece(piece)) {
    if (has_repeated_ascii_run(piece_str)) {
      return true;
    }
    if (piece.size() >= 3 && connector_count * 2 >= piece.size()) {
      return true;
    }
  }

  return false;
}

}  // namespace str_utils
}  // namespace sentencepiece