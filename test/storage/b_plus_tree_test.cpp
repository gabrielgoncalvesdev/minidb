#include "minidb/storage/index/b_plus_tree.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/storage/disk/disk_manager.hpp"

namespace minidb {
    namespace  {

        struct Int64Comparator {
            int operator()(const std::int64_t& a, const std::int64_t& b) const noexcept {
                return (a < b) ? -1 : ((a > b) ? 1 : 0);
            }
        };

        using Tree = BPlusTree<std::int64_t, RID, Int64Comparator>;
        RID MakeRid(std::int32_t p) { return RID{.page_id = p, .slot_num = 0}; }

        class BPlusTreeTest : public ::testing::Test {
            protected:
            void SetUp() override {
                path_ = (std::filesystem::temp_directory_path() / "minidb_bptree_test.db").string();
                std::filesystem::remove(path_);
            }
            void TearDown() override { std::filesystem::remove(path_); }
            std::string path_;
        };

        TEST_F(BPlusTreeTest, PointLookupSingleLeaf) {
            DiskManager dm(path_);
            BufferPoolManager bpm(100, &dm, 2);
            Tree tree(&bpm, Int64Comparator{}, 4, 4);

            EXPECT_FALSE(tree.GetValue(1).has_value());
            for (int k = 0; k < 3; ++k) {
                ASSERT_TRUE(tree.Insert(k, MakeRid(k)));
            }
            for (int k = 0; k < 3; ++k) {
                auto v = tree.GetValue(k);
                ASSERT_TRUE(v.has_value());
                EXPECT_EQ(*v, MakeRid(k));
            }

            EXPECT_FALSE(tree.GetValue(99).has_value());
            EXPECT_FALSE(tree.Insert(1, MakeRid(9)));
        }

        TEST_F(BPlusTreeTest, GrowsAndFindsAllInOrder) {
        DiskManager dm(path_);
        BufferPoolManager bpm(100, &dm, 2);
        Tree tree(&bpm, Int64Comparator{}, 4, 4);
        const int kN = 500;
        for (int k = 0; k < kN; ++k) {
            ASSERT_TRUE(tree.Insert(k, MakeRid(k)));
        }
        for (int k = 0; k < kN; ++k) {
            auto v = tree.GetValue(k);
            ASSERT_TRUE(v.has_value());
            EXPECT_EQ(*v, MakeRid(k));
        }
        EXPECT_FALSE(tree.GetValue(kN + 1).has_value());
        }

        TEST_F(BPlusTreeTest, FindsAllRandomOrder) {
        DiskManager dm(path_);
        BufferPoolManager bpm(100, &dm, 2);
        Tree tree(&bpm, Int64Comparator{}, 4, 4);
        const int kN = 500;
        std::vector<int> keys(kN);
        std::iota(keys.begin(), keys.end(), 0);
        std::mt19937 rng(42);
        std::shuffle(keys.begin(), keys.end(), rng);
        for (int k : keys) {
            ASSERT_TRUE(tree.Insert(k, MakeRid(k)));
        }
        for (int k = 0; k < kN; ++k) {
            ASSERT_TRUE(tree.GetValue(k).has_value());
        }
        }

        TEST_F(BPlusTreeTest, EmptyIteratorIsEnd) {
        DiskManager dm(path_);
        BufferPoolManager bpm(100, &dm, 2);
        Tree tree(&bpm, Int64Comparator{}, 4, 4);
        EXPECT_TRUE(tree.Begin() == tree.End());
    }

        TEST_F(BPlusTreeTest, OrderedScan) {
            DiskManager dm(path_);
            BufferPoolManager bpm(100, &dm, 2);
            Tree tree(&bpm, Int64Comparator{}, 4, 4);
            const int kN = 500;
            std::vector<int> keys(kN);
            std::iota(keys.begin(), keys.end(), 0);
            std::mt19937 rng(7);
            std::shuffle(keys.begin(), keys.end(), rng);
            for (int k : keys) {
                ASSERT_TRUE(tree.Insert(k, MakeRid(k)));
            }
            int expected = 0;
             for (auto it = tree.Begin(); it != tree.End(); ++it) {
            auto [k, v] = *it;              
            EXPECT_EQ(k, expected);
            EXPECT_EQ(v, MakeRid(expected));
            ++expected;
            }
            EXPECT_EQ(expected, kN);
        }
    }
}