#pragma once 

#include "minidb/catalog/catalog.hpp"
#include "minidb/execution/abstract_executor.hpp"
#include "minidb/execution/executor_context.hpp"
#include "minidb/storage/table/table_heap.hpp"
#include <optional>

namespace minidb {

    class SeqScanExecutor : public AbstractExecutor {
        public: 
        SeqScanExecutor(ExecutorContext* ctx, const std::string& table_name)
            : table_info_(ctx->GetCatalog()->GetTable(table_name)) {}

        void Init() override {
            iter_= table_info_->table->begin();
            end_ = table_info_->table->end();
        }

        bool Next(Tuple* out_tuple, RID* out_rid) override {
            if (!iter_.has_value() || *iter_ == *end_) {
                return false;
            }
            *out_tuple = **iter_;
            *out_rid = out_tuple->GetRid();
            ++(*iter_);
            return true;
        }

        [[nodiscard]] const Schema& GetOutputSchema() const override { return table_info_->schema; }

        private:
        TableInfo* table_info_;
        std::optional<TableHeap::Iterator> iter_;
        std::optional<TableHeap::Iterator> end_;
    };
}