#include "minidb/common/config.hpp"
#include "minidb/storage/table/table_page.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>


#include <gtest/gtest.h>

#include "minidb/catalog/column.hpp"
#include "minidb/catalog/schema.hpp"
#include "minidb/storage/table/tuple.hpp"
#include "minidb/type/value.hpp"


namespace minidb {
    namespace {

        Schema MakeSchema() {
            std::vector<Column> cols;
            cols.emplace_back("id", TypeId::kInteger);
            cols.emplace_back("name", TypeId::kVarchar);
            return Schema(std::move(cols));
        }

        Tuple MakeTuple(const Schema& s, std::int32_t id, const std::string& name) {
            std::vector<Value> vals;
            vals.emplace_back(id);
            vals.emplace_back(name);
            return Tuple(vals, s);
        }

        TEST(TablePageTest, InsertAndGet) {
            std::array<std::byte, PAGE_SIZE> buf{};
            TablePage tp(buf.data());
            tp.Init(7);
            EXPECT_EQ(tp.GetPageId(), 7);
            EXPECT_EQ(tp.GetNumTuples(), 0);

            Schema s = MakeSchema();
            auto slot = tp.InsertTuple(MakeTuple(s, 42, "gabriel"));
            ASSERT_TRUE(slot.has_value());
            EXPECT_EQ(*slot, 0);
            EXPECT_EQ(tp.GetNumTuples(), 1);

            auto got = tp.GetTuple(*slot);
            ASSERT_TRUE(got.has_value());
            EXPECT_EQ(got->GetValue(s, 0).GetInt(), 42);
            EXPECT_EQ(got->GetValue(s, 1).GetVarChar(), "gabriel");
            EXPECT_EQ(got->GetRid().page_id, 7);
            EXPECT_EQ(got->GetRid().slot_num, 0);
        }

        TEST(TablePageTest, MultipleTuples) {
            std::array<std::byte, PAGE_SIZE> buf{};
            TablePage tp(buf.data());
            tp.Init(1);
            Schema s = MakeSchema();
            for (std::int32_t i = 0; i<100; ++i) {
                ASSERT_TRUE(tp.InsertTuple(MakeTuple(s, i, "row" + std::to_string(i))).has_value());
            }
            EXPECT_EQ(tp.GetNumTuples(), 100);
            auto t = tp.GetTuple(50);
            ASSERT_TRUE(t.has_value());
            EXPECT_EQ(t->GetValue(s, 0).GetInt(), 50);
        }

        TEST(TablePageTest, MarkDeleteHidesTuple) {
            std::array<std::byte, PAGE_SIZE> buf{};
            TablePage tp(buf.data());
            tp.Init(1);
            Schema s = MakeSchema();
            auto slot = tp.InsertTuple(MakeTuple(s, 1, "x"));
            ASSERT_TRUE(slot.has_value());
            EXPECT_TRUE(tp.MarkDelete(*slot));
            EXPECT_FALSE(tp.GetTuple(*slot).has_value());
        }

        TEST(TablePageTest, PageFillsUp) {
            std::array<std::byte, PAGE_SIZE> buf{};
            TablePage tp(buf.data());
            tp.Init(1);
            Schema s = MakeSchema();
            int inserted = 0;
            while(tp.InsertTuple(MakeTuple(s, inserted, "some-name-here")).has_value()) {
                ++inserted;
                if (inserted > 100000) {
                    break;
                }
            }
            EXPECT_GT(inserted, 0);
            EXPECT_FALSE(tp.InsertTuple(MakeTuple(s, 0, "some-name-here")).has_value());
        }
    }
}




