#pragma once 

#include <atomic>
#include <cstdint>
#include <optional>
#include <utility>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/catalog/schema.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/storage/index/b_plus_tree.hpp"
#include "minidb/storage/table/tuple.hpp"
#include "minidb/type/value.hpp"

namespace minidb {

    struct IntKeyComparator {
        int operator()(const std::int64_t& a, const std::int64_t& b) const noexcept {
            return (a < b) ? -1 : ((a > b) ? 1 : 0);
        }
    };

    class BPlusTreeIndex {
        public: 
        BPlusTreeIndex(BufferPoolManager* bpm, Schema table_schema, std::atomic_uint32_t key_col)
        : tree_(bpm, IntKeyComparator{}, kLeafMax, kInternalMax),
        table_schema_(std::move(table_schema)),
        key_col_(key_col) {}

        void InsertEntry(const Tuple& tuple, RID rid) { tree_.Insert(ExtractKey(tuple), rid); }
        void DeleteEntry(const Tuple& tuple) { tree_.Remove(ExtractKey(tuple)); }
        [[nodiscard]] std::optional<RID> ScanKey(std::int64_t key) const { return tree_.GetValue(key); }
        [[nodiscard]] std::uint32_t GetKeyColumn() const noexcept { return key_col_; }

        private:
        static constexpr int kLeafMax = 250;
        static constexpr int kInternalMax = 330;

        [[nodiscard]] std::int64_t ExtractKey(const Tuple& tuple) const {
            const Value v = tuple.GetValue(table_schema_, key_col_);
            return (v.Type() == TypeId::kInteger) ? v.GetInt() : v.GetBigInt();
        }

        BPlusTree<std::int64_t, RID, IntKeyComparator> tree_;
        Schema table_schema_;
        std::uint32_t key_col_;
    };
}