#include "minidb/catalog/catalog.hpp"

#include <atomic>
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

        class CatalogTest : public ::testing::Test {
            protected: 
            void SetUp() override {
                path_ = (std::filesystem::temp_directory_path() / "minidb_catalog_test.db").string();
                std::filesystem::remove(path_);
            }
            void TearDown() override { std::filesystem::remove(path_); }
            std::string path_;
        };

        TEST_F(CatalogTest, CreateAndGetTable) {
            DiskManager dm(path_);
            BufferPoolManager bpm(200, &dm, 2);
            Catalog catalog(&bpm);
            TableInfo* ti = catalog.CreateTable("users", MakeSchema());
            ASSERT_NE(ti, nullptr);
            EXPECT_EQ(catalog.GetTable("users"), ti);
            EXPECT_EQ(catalog.CreateTable("users", MakeSchema()), nullptr);
            EXPECT_EQ(catalog.GetTable("nope"), nullptr);
        }

        TEST_F(CatalogTest, IndexPopulatesAndQueries) {
            DiskManager dm(path_);
            BufferPoolManager bpm(200, &dm, 2);
            Catalog catalog(&bpm);
            TableInfo* ti = catalog.CreateTable("users", MakeSchema());
            ASSERT_NE(ti, nullptr);
            const int kN = 200;
            for (int i = 0; i < kN; ++i) {
                ASSERT_TRUE(ti->table->InsertTuple(MakeTuple(ti->schema, i, "u" + std::to_string(i))).has_value());
            }
            IndexInfo* ii = catalog.CreateIndex("users_by_id", "users", 0);
            ASSERT_NE(ii, nullptr);
            for (int i  = 0; i < kN; ++i) {
                auto rid = ii->index->ScanKey(static_cast<std::int64_t>(i));
                ASSERT_TRUE(rid.has_value());
                auto t = ti->table->GetTuple(*rid);
                ASSERT_TRUE(t.has_value());
                EXPECT_EQ(t->GetValue(ti->schema, 0).GetInt(), i);
            }
            EXPECT_FALSE(ii->index->ScanKey(static_cast<std::int64_t>(9999)).has_value());
        }

        TEST_F(CatalogTest, DeleteViaIndex) {
        DiskManager dm(path_);
        BufferPoolManager bpm(200, &dm, 2);
        Catalog catalog(&bpm);
        TableInfo* ti = catalog.CreateTable("users", MakeSchema());
        ASSERT_NE(ti, nullptr);
        for (int i = 0; i < 10; ++i) {
            ASSERT_TRUE(ti->table->InsertTuple(MakeTuple(ti->schema, i, "u")).has_value());
        }
        IndexInfo* ii = catalog.CreateIndex("idx", "users", 0);
        ASSERT_NE(ii, nullptr);
        auto rid5 = ii->index->ScanKey(static_cast<std::int64_t>(5));
        ASSERT_TRUE(rid5.has_value());
        EXPECT_TRUE(ti->table->MarkDelete(*rid5));
        ii->index->DeleteEntry(MakeTuple(ti->schema, 5, "x"));  
        EXPECT_FALSE(ii->index->ScanKey(static_cast<std::int64_t>(5)).has_value());
        EXPECT_TRUE(ii->index->ScanKey(static_cast<std::int64_t>(6)).has_value());
        }
    }
}