#include "minidb/execution/execution_engine.hpp"
#include "minidb/execution/filter_executor.hpp"
#include "minidb/execution/seq_scan_executor.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/catalog/catalog.hpp"
#include "minidb/catalog/column.hpp"
#include "minidb/catalog/schema.hpp"
#include "minidb/execution/executor_context.hpp"
#include "minidb/execution/expressions/column_value_expression.hpp"
#include "minidb/execution/expressions/comparison_expression.hpp"
#include "minidb/execution/expressions/constant_value_expression.hpp"
#include "minidb/storage/disk/disk_manager.hpp"
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
        Tuple MakeTuple(const Schema& s, std::int32_t id, std::int32_t age) {
            std::vector<Value> vals;
            vals.emplace_back(id);
            vals.emplace_back(age);
            return Tuple(vals, s);
        }

        class ExecutorTest : public ::testing::Test {
            public:
            void SetUp() override {
                path_ = (std::filesystem::temp_directory_path() / "minidb_exec_test.db").string();
                std::filesystem::remove(path_);
            }
            void TearDown() override { std::filesystem::remove(path_); }
            std::string path_;
        };

        TEST_F(ExecutorTest, SeqScanReadsAll) {
            DiskManager dm(path_);
            BufferPoolManager bpm(200, &dm, 2);
            Catalog catalog(&bpm);
            TableInfo* ti = catalog.CreateTable("t", MakeSchema());
            const int kN = 100;
            for (int i = 0; i < kN; ++i) {
                ASSERT_TRUE(ti->table->InsertTuple(MakeTuple(ti->schema, i, i)).has_value());
            }
            ExecutorContext ctx(&catalog, &bpm);
            SeqScanExecutor scan(&ctx, "t");
            const std::vector<Tuple> rows = ExecutionEngine::Execute(scan);
            EXPECT_EQ(rows.size(), static_cast<std::size_t>(kN));
            long long sum = 0;
            for (const Tuple& r : rows) {
                sum+= r.GetValue(ti->schema, 1).GetInt();
            }
            EXPECT_EQ(sum, static_cast<long long>(kN) * (kN - 1) / 2);
        }

        TEST_F(ExecutorTest, FilterKeepsMatching) {
            DiskManager dm(path_);
            BufferPoolManager bpm(200,  &dm, 2);
            Catalog catalog(&bpm);
            TableInfo* ti = catalog.CreateTable("t", MakeSchema());
            ASSERT_NE(ti, nullptr);
            for (int i = 0; i < 100; ++i) {
                ASSERT_TRUE(ti->table->InsertTuple(MakeTuple(ti->schema, i, i)).has_value());
            }
            ExecutorContext ctx(&catalog, &bpm);
            auto scan = std::make_unique<SeqScanExecutor>(&ctx, "t");
            auto pred = std::make_unique<ComparisonExpression>(std::make_unique<ColumnValueExpression>(1),                       // coluna age
        std::make_unique<ConstantValueExpression>(Value(std::int32_t{25})),
        ComparisonType::kGreaterThan);
        FilterExecutor filter(std::move(scan), std::move(pred));
        const std::vector<Tuple> rows = ExecutionEngine::Execute(filter);
        EXPECT_EQ(rows.size(), 74U);  
        for (const Tuple& r : rows) { EXPECT_GT(r.GetValue(ti->schema, 1).GetInt(), 25); }
        }
    }
}