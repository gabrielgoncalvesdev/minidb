// test/common/version_test.cpp
#include "minidb/common/version.hpp"

#include <gtest/gtest.h>

TEST(VersionTest, LooksLikeSemVer) {
  const std::string_view v = minidb::Version();
  EXPECT_FALSE(v.empty());
  EXPECT_TRUE(v.contains('.'));
}