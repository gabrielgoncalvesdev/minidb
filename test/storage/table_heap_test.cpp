#include "minidb/storage/table/table_heap.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/catalog/column.hpp"
#include "minidb/catalog/schema.hpp"
#include "minidb/storage/disk/disk_manager.hpp"
#include "minidb/type/value.hpp"

namespace minidb {
    namespace  {
    
        class TableHeapTest : public ::testing::Test {
            protected:
            void SetUp() override {
                path_ = (std::filesystem::temp_directory_path() / "minidb_heap_test.db").string();
                std::filesystem::remove(path_);
            }
            void TearDown() override { std::filesystem::remove(path_); }
            std::string path_;

            static Schema MakeSchema() {
                std::vector<Column> cols;
                cols.emplace_back("id", TypeId::kInteger);
                cols.emplace_back("name", TypeId::kVarchar);
                return Schema(std::move(cols));
            }
            static Tuple MakeTuple(const Schema& s, std::int32_t id, const std::string& name) {
                std::vector<Value> vals;
                vals.emplace_back(id);
                vals.emplace_back(name);
                return Tuple(vals, s);
            }
        };

        TEST_F(TableHeapTest, InsertAndGetByRid) {
            DiskManager dm(path_);
            BufferPoolManager bpm(10, &dm, 2);
            TableHeap heap(&bpm);
            Schema s = MakeSchema();
            auto rid = heap.InsertTuple(MakeTuple(s, 1, "alice"));
            ASSERT_TRUE(rid.has_value());
            auto got = heap.GetTuple(*rid);
            ASSERT_TRUE(got.has_value());
            EXPECT_EQ(got->GetValue(s, 0).GetInt(), 1);
            EXPECT_EQ(got->GetValue(s, 1).GetVarChar(), "alice");
        }

        TEST_F(TableHeapTest, ScanAllAcrossPages ) {
            DiskManager dm(path_);
            BufferPoolManager bpm(10, &dm, 2);
            TableHeap heap(&bpm);
            Schema s = MakeSchema();

            const int kn = 1000;
            for (int i = 0; i < kn; ++i) {
                ASSERT_TRUE(heap.InsertTuple(MakeTuple(s, i, "row-" + std::to_string(i))).has_value());
            }
            long long sum = 0;
            int count = 0;
            for (auto it = heap.begin(); it != heap.end(); ++it) {
            sum += (*it).GetValue(s, 0).GetInt();
            ++count;
        }
        EXPECT_EQ(count, kn);
        EXPECT_EQ(sum, static_cast<long long>(kn) * (kn - 1) / 2);  // 0+1+...+(N-1)
        }

        TEST_F(TableHeapTest, DeleteHidesFromScan) {
        DiskManager dm(path_);
        BufferPoolManager bpm(10, &dm, 2);
        TableHeap heap(&bpm);
        Schema s = MakeSchema();

        auto r0 = heap.InsertTuple(MakeTuple(s, 10, "x"));
        auto r1 = heap.InsertTuple(MakeTuple(s, 20, "y"));
        ASSERT_TRUE(r0.has_value());
        ASSERT_TRUE(r1.has_value());
        EXPECT_TRUE(heap.MarkDelete(*r0));

        int count = 0;
        for (auto it = heap.begin(); it != heap.end(); ++it) {
            ++count;
        }
        EXPECT_EQ(count, 1);
        EXPECT_FALSE(heap.GetTuple(*r0).has_value());
        EXPECT_TRUE(heap.GetTuple(*r1).has_value());
        }
    }
}