#include "minidb/storage/index/b_plus_tree_internal_page.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "minidb/common/config.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/storage/index/b_plus_tree_leaf_page.hpp"

namespace minidb {
    namespace {
        struct Int64Comparator {
            int operator()(const std::int64_t& a, const std::int64_t& b) const noexcept {
                return (a < b) ? -1 : ((a > b) ? 1 : 0);
            }
        };

        using Internal = BPlusTreeInternalPage<std::int64_t, Int64Comparator>;
        using Leaf = BPlusTreeLeafPage<std::int64_t, RID, Int64Comparator>;
        RID MakeRid(std::int32_t p) {return RID{.page_id = p, .slot_num = 0}; }

        TEST(BPlusTreeInternalPageTest, PopulateNewRootAndRoute) {
            std::array<std::byte, PAGE_SIZE> buf{};
            Internal node(buf.data());
            node.Init(4);
            Int64Comparator cmp;

            node.PopulateNewRoot(100, 50, 200);
            EXPECT_EQ(node.GetSize(), 2);
            EXPECT_EQ(node.ValueAt(0), 100);
            EXPECT_EQ(node.KeyAt(1), 50);
            EXPECT_EQ(node.ValueAt(1), 200);

            node.InsertAfter(200, 80, 300);
            EXPECT_EQ(node.GetSize(), 3);
            EXPECT_EQ(node.LookUp(40, cmp), 100);
            EXPECT_EQ(node.LookUp(50, cmp), 200);
            EXPECT_EQ(node.LookUp(70, cmp), 200);
            EXPECT_EQ(node.LookUp(80, cmp), 300); 
            EXPECT_EQ(node.LookUp(90, cmp), 300);
        }

        TEST(BPlusTreeInternalPageTest, SplitMovesUpperHalf) {
            
            std::array<std::byte, PAGE_SIZE> buf{};
            Internal node(buf.data());
            node.Init(10);
            node.SetValueAt(0, 1000);
            node.SetKeyAt(1, 10); node.SetValueAt(1, 1001);
            node.SetKeyAt(2, 20); node.SetValueAt(2, 1002);
            node.SetKeyAt(3, 30); node.SetValueAt(3, 1003);
            node.SetKeyAt(4, 40); node.SetValueAt(4, 1004);
            node.SetSize(5);

            std::array<std::byte, PAGE_SIZE> buf2{};
            Internal other(buf2.data());
            other.Init(10);
            const int half = node.GetSize() / 2;
            const std::int64_t push = node.KeyAt(half);
            node.MoveHalfTo(other);

            EXPECT_EQ(push, 20);              // a chave do meio sobe pro pai
            EXPECT_EQ(node.GetSize(), 2);
            EXPECT_EQ(other.GetSize(), 3);
            EXPECT_EQ(other.ValueAt(0), 1002);
            EXPECT_EQ(other.KeyAt(1), 30);
            EXPECT_EQ(other.KeyAt(2), 40);
        }

        TEST(BPlusTreeInternalPageTest, LeafSplitMovesHalf) {
        std::array<std::byte, PAGE_SIZE> buf{};
        Leaf leaf(buf.data());
        leaf.Init(100);
        Int64Comparator cmp;
        for (int k = 0; k < 10; ++k) {
            leaf.Insert(k, MakeRid(k), cmp);
        }
        std::array<std::byte, PAGE_SIZE> buf2{};
        Leaf other(buf2.data());
        other.Init(100);
        leaf.MoveHalfTo(other);

        EXPECT_EQ(leaf.GetSize(), 5);
        EXPECT_EQ(other.GetSize(), 5);
        EXPECT_EQ(leaf.KeyAt(0), 0);
        EXPECT_EQ(other.KeyAt(0), 5);
        }
    }
}