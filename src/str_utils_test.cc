#include "str_utils.h"

#include "testharness.h"
#include "util.h"

namespace sentencepiece {
namespace str_utils {

TEST(StrUtilsTest, IsMalformedFiltersVideoIdsAndPromoPhrases) {
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("av32225766")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("BV1SM4y1x7AB")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("asdf")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("qwerty")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("请关注我")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("给视频点赞")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("主页收藏有惊喜")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("直播间传送门")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("搬运自")));
  EXPECT_TRUE(is_malformed(string_util::UTF8ToUnicodeText("如侵删")));
}

TEST(StrUtilsTest, IsMalformedKeepsLegitPhrases) {
  EXPECT_FALSE(is_malformed(string_util::UTF8ToUnicodeText("原神，启动")));
  EXPECT_FALSE(is_malformed(string_util::UTF8ToUnicodeText("黑神话：悟空")));
  EXPECT_FALSE(is_malformed(string_util::UTF8ToUnicodeText("收藏家")));
  EXPECT_FALSE(is_malformed(string_util::UTF8ToUnicodeText("直播间")));
}

}  // namespace str_utils
}  // namespace sentencepiece