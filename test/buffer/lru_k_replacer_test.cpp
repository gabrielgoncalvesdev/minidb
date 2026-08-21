#include "minidb/buffer/lru_k_replacer.hpp"

#include <gtest/gtest.h>

namespace minidb {
    namespace  {
    
        TEST(LRUKReplacerTest, BasicEvictionOrder) {
            LRUKReplacer replacer(7, 2);
            for (int i = 1; i <= 6; ++i) {
                replacer.RecordAccess(i);
            }
            for (int i = 1; i <= 5; ++i) {
                replacer.SetEvictable(i, true);
            }
            replacer.SetEvictable(6, false);
            EXPECT_EQ(replacer.Size(), 5U);

            replacer.RecordAccess(1);

            EXPECT_EQ(replacer.Evict(), 2);
            EXPECT_EQ(replacer.Evict(), 3);
            EXPECT_EQ(replacer.Evict(), 4);
            EXPECT_EQ(replacer.Size(), 2U);
            EXPECT_EQ(replacer.Evict(), 5);  // 5 (inf) antes de 1 (finito)
            EXPECT_EQ(replacer.Evict(), 1);
            EXPECT_FALSE(replacer.Evict().has_value());
        }

        TEST(LRUKReplacerTest, InfiniteBeatsFinite) {
            LRUKReplacer replacer(3, 2);
            replacer.RecordAccess(0);
            replacer.RecordAccess(0); // frame 0 com dois acessos finito
            replacer.RecordAccess(1); // frame 1 com um acesso infinito
            replacer.SetEvictable(0, true);
            replacer.SetEvictable(1, true);
            EXPECT_EQ(replacer.Evict(), 1);
            EXPECT_EQ(replacer.Evict(), 0);
            EXPECT_FALSE(replacer.Evict().has_value());
        }

        TEST(LRUKReplacerTest, EvictableTogglingAffectsSize) {
            LRUKReplacer replacer(3, 2);
            replacer.RecordAccess(0);
            EXPECT_EQ(replacer.Size(), 0u);
            replacer.SetEvictable(0, true);
            EXPECT_EQ(replacer.Size(), 1U);
            replacer.SetEvictable(0, false);
            EXPECT_EQ(replacer.Size(), 0u);
            EXPECT_FALSE(replacer.Evict().has_value());
        }

        TEST(LRUKReplacerTest, RemoveFrame) {
            LRUKReplacer replacer(3, 2);
            replacer.RecordAccess(0);
            replacer.RecordAccess(1);
            replacer.SetEvictable(0, true);
            replacer.SetEvictable(1, true);
            EXPECT_EQ(replacer.Size(), 2U);
            replacer.Remove(0);
            EXPECT_EQ(replacer.Size(), 1U);
            EXPECT_EQ(replacer.Evict(), 1);
            EXPECT_FALSE(replacer.Evict().has_value());
        }

        TEST(LRUKReplacerTest, EvictOnEmptyReturnsNullOpt) {
            LRUKReplacer replacer(3, 2);
            EXPECT_FALSE(replacer.Evict().has_value());
        }
    }
}