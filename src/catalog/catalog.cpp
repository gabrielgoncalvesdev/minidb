#include "minidb/catalog/catalog.hpp"

#include <memory>
#include <utility>

#include "minidb/storage/index/b_plus_tree_index.hpp"
#include "minidb/storage/table/table_heap.hpp"
#include "minidb/storage/table/tuple.hpp"

namespace minidb {
    TableInfo* Catalog::CreateTable(const std::string& name, Schema schema) {
        if (tables_.find(name) != tables_.end()) { return nullptr; }
        
        TableInfo info{.oid = next_oid_++,
        .name = name,
        .schema = std::move(schema),
        .table = std::make_unique<TableHeap>(bpm_)
        };
        auto holder = std::make_unique<TableInfo>(std::move(info));
        TableInfo* ptr = holder.get();
        tables_[name] = std::move(holder);
        return ptr;
    }

    TableInfo* Catalog::GetTable(const std::string& name) const {
        auto it = tables_.find(name);
        return (it == tables_.end() ? nullptr : it->second.get());
    }

    IndexInfo* Catalog::CreateIndex(const std::string& index_name, const std::string& table_name, std::uint32_t key_col) {
      if (indexes_.find(index_name) != indexes_.end()) { return  nullptr; }
      TableInfo* table_info = GetTable(table_name);
      if (table_info == nullptr) { return nullptr; }
      
      IndexInfo info{.name = index_name, .table_name = table_name, .key_col = key_col, .index = 
        std::make_unique<BPlusTreeIndex>(bpm_, table_info->schema, key_col)};

        auto* table_heap = table_info->table.get();

        for (auto it = table_heap->begin(); it != table_heap->end(); ++it) {
            Tuple tuple = *it;
            info.index->InsertEntry(tuple, tuple.GetRid());   
        }
        auto holder = std::make_unique<IndexInfo>(std::move(info));
            IndexInfo* ptr = holder.get();
            indexes_[index_name] = std::move(holder);
            return ptr;
    }

    IndexInfo* Catalog::GetIndex(const std::string& name) const {
        auto it = indexes_.find(name);
        return (it == indexes_.end() ? nullptr : it->second.get());
    }
    }