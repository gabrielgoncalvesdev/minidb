#pragma once 

#include <memory>
#include <utility>

#include "minidb/catalog/schema.hpp"
#include "minidb/execution/abstract_executor.hpp"
#include "minidb/execution/expressions/abstract_expression.hpp"

namespace minidb {

    class FilterExecutor : public AbstractExecutor {
        public:
        FilterExecutor(std::unique_ptr<AbstractExecutor> child,
                        std::unique_ptr<AbstractExpression> predicate) 
        : child_(std::move(child)), predicate_(std::move(predicate)) {}

        void Init() override { child_->Init(); }

        bool Next(Tuple* out_tuple, RID* out_rid) override {
            Tuple t;
            RID rid;
            while (child_->Next(&t, &rid)) {
               const Value keep = predicate_->Evaluate(t, child_->GetOutputSchema());
                if (!keep.IsNull() && keep.GetBool()) {
                    *out_tuple = t;
                    *out_rid = rid;
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] const Schema& GetOutputSchema() const override {
            return child_->GetOutputSchema();
        }

        private:
        std::unique_ptr<AbstractExecutor> child_;
        std::unique_ptr<AbstractExpression> predicate_;
    };
}