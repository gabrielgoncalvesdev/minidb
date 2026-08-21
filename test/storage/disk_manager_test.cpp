// test/storage/disk_manager_test.cpp
#include "minidb/storage/disk/disk_manager.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

namespace minidb {
namespace {

class DiskManagerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    path_ = (std::filesystem::temp_directory_path() / "minidb_disk_test.db").string();
    std::filesystem::remove(path_);
  }
  void TearDown() override { std::filesystem::remove(path_); }
  std::string path_;
};

std::array<std::byte, PAGE_SIZE> MakePage(std::byte fill) {
  std::array<std::byte, PAGE_SIZE> page{};
  page.fill(fill);
  return page;
}

TEST_F(DiskManagerTest, WriteThenReadRoundTrips) {
  DiskManager dm(path_);
  const page_id_t id = dm.AllocatePage();
  EXPECT_EQ(id, 0);

  const auto written = MakePage(std::byte{0xAB});
  ASSERT_TRUE(dm.WritePage(id, written).has_value());

  std::array<std::byte, PAGE_SIZE> read_back{};
  ASSERT_TRUE(dm.ReadPage(id, read_back).has_value());
  EXPECT_EQ(written, read_back);
}

TEST_F(DiskManagerTest, AllocateIncrementsPageCount) {
  DiskManager dm(path_);
  EXPECT_EQ(dm.NumPages(), 0);
  EXPECT_EQ(dm.AllocatePage(), 0);
  EXPECT_EQ(dm.AllocatePage(), 1);
  EXPECT_EQ(dm.NumPages(), 2);
}

TEST_F(DiskManagerTest, ReadOutOfRangeReturnsError) {
  DiskManager dm(path_);
  std::array<std::byte, PAGE_SIZE> buf{};
  const auto result = dm.ReadPage(0, buf);
  ASSERT_FALSE(result.has_value());
  EXPECT_EQ(result.error(), Status::kPageOutOfRange);
}

TEST_F(DiskManagerTest, AllocatedButUnwrittenPageReadsAsZeros) {
  DiskManager dm(path_);
  const page_id_t id = dm.AllocatePage();
  auto buf = MakePage(std::byte{0xFF});
  ASSERT_TRUE(dm.ReadPage(id, buf).has_value());
  EXPECT_EQ(buf, MakePage(std::byte{0}));
}

TEST_F(DiskManagerTest, DataPersistsAcrossReopen) {
  const auto written = MakePage(std::byte{0x7E});
  {
    DiskManager dm(path_);
    ASSERT_EQ(dm.AllocatePage(), 0);
    ASSERT_TRUE(dm.WritePage(0, written).has_value());
    dm.Sync();
  }
  DiskManager dm2(path_);
  EXPECT_EQ(dm2.NumPages(), 1);
  std::array<std::byte, PAGE_SIZE> read_back{};
  ASSERT_TRUE(dm2.ReadPage(0, read_back).has_value());
  EXPECT_EQ(written, read_back);
}

}  // namespace
}  // namespace minidb
