#pragma once 

#include <memory>
#include <utility>

#include "minidb/execution/expressions/abstract_expression.hpp"

namespace minidb {

    enum class ComparisonType { kEqual, kNotEqual, kLessThan, kGreaterThan, kLessEqual, kGreaterEqual };

    class ComparisonExpression : public AbstractExpression {
        public:
        ComparisonExpression(std::unique_ptr<AbstractExpression> left, std::unique_ptr<AbstractExpression> right, ComparisonType type)
            : left_(std::move(left)), right_(std::move(right)), type_(type) {}

        [[nodiscard]] Value Evaluate(const Tuple& tuple, const Schema& schema) const override {
            const Value l = left_->Evaluate(tuple, schema);
            const Value r = right_->Evaluate(tuple, schema);
            bool result = false;
            switch (type_) {
                case ComparisonType::kEqual:  result = l.Equals(r);  break;
                case ComparisonType::kNotEqual: result = !l.Equals(r); break;
                case ComparisonType::kLessThan: result = l.LessThan(r); break;
                case ComparisonType::kGreaterThan: result = r.LessThan(l); break;
                case ComparisonType::kLessEqual: result = l.Equals(r) || l.LessThan(r); break;
                case ComparisonType::kGreaterEqual: result = r.LessThan(l) || r.Equals(l); break;
            }
            return Value(result);
        }
        private:
        std::unique_ptr<AbstractExpression> left_;
        std::unique_ptr<AbstractExpression> right_;
        ComparisonType type_;
    };
}

