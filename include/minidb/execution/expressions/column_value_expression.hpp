#pragma once 

#include <cstdint>

#include "minidb/catalog/column.hpp"
#include "minidb/execution/expressions/abstract_expression.hpp"

namespace minidb {

    class ColumnValueExpression : public AbstractExpression {
        public: 
        explicit ColumnValueExpression(std::uint32_t col_idx) : col_idx_(col_idx) {}

        [[nodiscard]] Value Evaluate(const Tuple& tuple, const Schema& schema) const override {
            return tuple.GetValue(schema, col_idx_);
        }

        private:
        std::uint32_t col_idx_;
    };
}