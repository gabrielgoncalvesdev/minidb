#include "minidb/execution/expressions/abstract_expression.hpp"
#include "minidb/execution/expressions/column_value_expression.hpp"
#include "minidb/execution/expressions/comparison_expression.hpp"
#include "minidb/execution/expressions/constant_value_expression.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>
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
            cols.emplace_back("age", TypeId::kInteger);
            return Schema(std::move(cols));
        }
        Tuple MakeTuple(const Schema& s) {
            std::vector<Value> vals;
            vals.emplace_back(std::int32_t{5});
            vals.emplace_back(std::int32_t{30});
            return Tuple(vals, s);
        }

        std::unique_ptr<AbstractExpression> Col(std::uint32_t i) {
            return std::make_unique<ColumnValueExpression>(i);
        }
        std::unique_ptr<AbstractExpression> Const(std::int32_t v) {
            return std::make_unique<ConstantValueExpression>(Value(v));
            }
        std::unique_ptr<AbstractExpression> Cmp(std::unique_ptr<AbstractExpression> l,
                                                    std::unique_ptr<AbstractExpression> r, ComparisonType t) {
            return std::make_unique<ComparisonExpression>(std::move(l), std::move(r), t);
        }

        TEST(ExpressionTest, ColumnAndConstant) {
            Schema s  = MakeSchema();
            Tuple t = MakeTuple(s);
            EXPECT_EQ(ColumnValueExpression(0).Evaluate(t, s).GetInt(), 5);
            EXPECT_EQ(ColumnValueExpression(1).Evaluate(t, s).GetInt(), 30);
            EXPECT_EQ(ConstantValueExpression(Value(std::int32_t{42})).Evaluate(t, s).GetInt(), 42);
        }
        TEST(ExpressionTest, AllComparisons) {
            Schema s = MakeSchema();
            Tuple t = MakeTuple(s);  // id=5, age=30
            EXPECT_TRUE(Cmp(Col(1), Const(30), ComparisonType::kEqual)->Evaluate(t, s).GetBool());
            EXPECT_FALSE(Cmp(Col(1), Const(31), ComparisonType::kEqual)->Evaluate(t, s).GetBool());
            EXPECT_FALSE(Cmp(Col(0), Const(5), ComparisonType::kNotEqual)->Evaluate(t, s).GetBool());
            EXPECT_TRUE(Cmp(Col(0), Const(10), ComparisonType::kLessThan)->Evaluate(t, s).GetBool());
            EXPECT_FALSE(Cmp(Col(0), Const(10), ComparisonType::kGreaterThan)->Evaluate(t, s).GetBool());
            EXPECT_TRUE(Cmp(Col(0), Const(5), ComparisonType::kLessEqual)->Evaluate(t, s).GetBool());
            EXPECT_TRUE(Cmp(Col(0), Const(5), ComparisonType::kGreaterEqual)->Evaluate(t, s).GetBool());
            EXPECT_TRUE(Cmp(Col(0), Const(3), ComparisonType::kGreaterThan)->Evaluate(t, s).GetBool());
        }
    }
}