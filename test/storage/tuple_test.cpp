#include "minidb/storage/table/tuple.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "minidb/catalog/column.hpp"
#include "minidb/catalog/schema.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/type/value.hpp"

#include <gtest/gtest.h>

namespace minidb {
    namespace {

        Schema MakeSchema() {
            std::vector<Column> cols;
            cols.emplace_back("id", TypeId::kInteger);
            cols.emplace_back("score", TypeId::kBigInt);
            cols.emplace_back("name", TypeId:: kVarchar);
            cols.emplace_back("active", TypeId::kBoolean);
            return Schema(std::move(cols));
        }

        TEST(SchemaTest, ComputesOffsetsAndSize) {
            Schema s = MakeSchema();
            EXPECT_EQ(s.GetColumnCount(), 4U);
            EXPECT_EQ(s.GetColumn(0).Offset(), 0U);
            EXPECT_EQ(s.GetColumn(1).Offset(), 4U);
            EXPECT_EQ(s.GetColumn(2).Offset(), 12U);
            EXPECT_EQ(s.GetColumn(3).Offset(), 16U);
            EXPECT_EQ(s.InlinedSize(), 17U);
            ASSERT_EQ(s.UninlinedColumns().size(), 1U);
            EXPECT_EQ(s.UninlinedColumns()[0], 2U);
        }

        TEST(TupleTest, RoundTripsAllColumns) {
            Schema s = MakeSchema();
            std::vector<Value> vals;
            vals.emplace_back(std::int32_t{7});
            vals.emplace_back(std::int64_t{1000});
            vals.emplace_back(std::string{"gabriel"});
            vals.emplace_back(true);
            Tuple t(vals, s);
            EXPECT_EQ(t.GetValue(s, 0).GetInt(), 7);
            EXPECT_EQ(t.GetValue(s, 1).GetBigInt(), 1000);
            EXPECT_EQ(t.GetValue(s, 2).GetVarChar(), "gabriel");
            EXPECT_EQ(t.GetValue(s, 3).GetBool(), true);
        }

        TEST(TupleTest, HandlesMultipleVarchars) {
            std::vector<Column> cols;
            cols.emplace_back("a", TypeId::kVarchar);
            cols.emplace_back("n", TypeId::kInteger);
            cols.emplace_back("b", TypeId::kVarchar);
            Schema s(std::move(cols));
            std::vector<Value> vals;
            vals.emplace_back(std::string{"hello"});
            vals.emplace_back(std::int32_t{42});
            vals.emplace_back(std::string{"world"});
            Tuple t(vals, s);
            EXPECT_EQ(t.GetValue(s, 0).GetVarChar(), "hello");
            EXPECT_EQ(t.GetValue(s, 1).GetInt(), 42);
            EXPECT_EQ(t.GetValue(s, 2).GetVarChar(), "world");
        }

        TEST(TupleTest, EmptyVarcharWorks) {
            std::vector<Column> cols;
            cols.emplace_back("s", TypeId::kVarchar);
            Schema s(std::move(cols));
            std::vector<Value> vals;
            vals.emplace_back(std::string{""});
            Tuple t(vals, s);
            EXPECT_EQ(t.GetValue(s, 0).GetVarChar(), "");
            }

        TEST(TupleTest, RidRoundTrip) {
            Schema s = MakeSchema();
            std::vector<Value> vals;
            vals.emplace_back(std::int32_t{1});
            vals.emplace_back(std::int64_t{2});
            vals.emplace_back(std::string{"x"});
            vals.emplace_back(false);
            Tuple t(vals, s);
            t.SetRid(RID{.page_id = 3, .slot_num = 9});
            EXPECT_EQ(t.GetRid().page_id, 3);
            EXPECT_EQ(t.GetRid().slot_num, 9);
        }
    }
}