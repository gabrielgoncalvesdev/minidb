#include "minidb/common/config.hpp"
#include "minidb/storage/index/b_plus_tree_leaf_page.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "minidb/common/rid.hpp"

namespace minidb {
    namespace {

        struct Int64Comparator {
            int operator()(const std::int64_t& a, const std::int64_t& b) const noexcept {
                return (a < b) ? -1 : ((a > b) ? 1 : 0);
            }
        };

        using LeafPage = BPlusTreeLeafPage<std::int64_t, RID, Int64Comparator>;

        RID MakeRid(std::int32_t p) { return RID{.page_id = p, .slot_num = 0}; }

        TEST(BPlusTreeLeafPageTest, InsertKeepsSorted) {
            std::array<std::byte, PAGE_SIZE> buf{};
            LeafPage leaf(buf.data());
            leaf.Init(3); //conferir aqui 
            Int64Comparator cmp;

            EXPECT_EQ(leaf.GetSize(), 0);
            EXPECT_TRUE(leaf.Insert(30, MakeRid(3), cmp));
            EXPECT_TRUE(leaf.Insert(10, MakeRid(1), cmp));
            EXPECT_TRUE(leaf.Insert(20, MakeRid(2), cmp));
            EXPECT_EQ(leaf.GetSize(), 3);
            EXPECT_EQ(leaf.KeyAt(0), 10);
            EXPECT_EQ(leaf.KeyAt(1), 20);
            EXPECT_EQ(leaf.KeyAt(2), 30);
            EXPECT_EQ(leaf.ValueAt(0), MakeRid(1));
        }

        TEST(BPlusTreeLeafPageTest, LookupFindsAndMisses) {
            std::array<std::byte, PAGE_SIZE> buf{};
            LeafPage leaf(buf.data());
            leaf.Init(10);
            Int64Comparator cmp;
            leaf.Insert(5, MakeRid(50), cmp);
            leaf.Insert(15, MakeRid(150), cmp );

            auto v = leaf.Lookup(15, cmp);
            ASSERT_TRUE(v.has_value());
            EXPECT_EQ(v, MakeRid(150));
            EXPECT_FALSE(leaf.Lookup(99, cmp).has_value());
        }

        TEST(BPlusTreeLeafPageTest, RejectsDuplicates) {
            std::array<std::byte, PAGE_SIZE> buf {};
            LeafPage leaf(buf.data());
            leaf.Init(10);
            Int64Comparator cmp;
            EXPECT_TRUE(leaf.Insert(7, MakeRid(1), cmp));
            EXPECT_FALSE(leaf.Insert(7, MakeRid(2), cmp));
            EXPECT_EQ(leaf.GetSize(), 1);
        }

        TEST(BPlusTreeLeafPageTest, ManyKeysStaySorted) {
            std::array<std::byte, PAGE_SIZE> buf{};
            LeafPage leaf(buf.data());
            leaf.Init(10);
            Int64Comparator cmp;
            const int keys[] = {7, 3, 9, 1, 5, 8, 2, 6, 4, 0};
            for (int k : keys) {
                ASSERT_TRUE(leaf.Insert(k, MakeRid(k), cmp));
            }
            EXPECT_EQ(leaf.GetSize(), 10);
            for (int i = 0; i < 10; ++i) {
                EXPECT_EQ(leaf.KeyAt(i), i);
            }
        }
    }
}

