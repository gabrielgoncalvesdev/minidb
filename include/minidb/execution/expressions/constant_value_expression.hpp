#pragma once 

#include <utility>

#include "minidb/execution/expressions/abstract_expression.hpp"
#include "minidb/type/value.hpp"

namespace minidb {

    class ConstantValueExpression : public AbstractExpression {
        public: 
        explicit ConstantValueExpression(Value value) : value_(std::move(value)) {}

        [[nodiscard]] Value Evaluate(const Tuple& /*tuple*/, const Schema& /*schema*/) const override {
            return value_;
        }

        private:
        Value value_;
    };
}