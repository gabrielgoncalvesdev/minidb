#pragma once 

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>


#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/catalog/schema.hpp"
#include "minidb/storage/index/b_plus_tree_index.hpp"
#include "minidb/storage/table/table_heap.hpp"

namespace minidb {

    using table_oid_t = std::int32_t;

    struct TableInfo {
        table_oid_t oid;
        std::string name;
        Schema schema;
        std::unique_ptr<TableHeap> table;
    };

    struct IndexInfo {
        std::string name;
        std::string table_name;
        std::uint32_t key_col;
        std::unique_ptr<BPlusTreeIndex> index;
    };

    class Catalog {
        public:
        explicit Catalog(BufferPoolManager* bpm) : bpm_(bpm) {}

        TableInfo* CreateTable(const std::string& name, Schema schema);
        [[nodiscard]] TableInfo* GetTable(const std::string& name) const;

        IndexInfo* CreateIndex(const std::string& name, const std::string& table_name, std::uint32_t key_col);
        [[nodiscard]] IndexInfo* GetIndex(const std::string& name) const;

        private:
        BufferPoolManager* bpm_;
        std::unordered_map<std::string, std::unique_ptr<TableInfo>> tables_;
        std::unordered_map<std::string, std::unique_ptr<IndexInfo>> indexes_;
        table_oid_t next_oid_ = 0;
    };
}