#pragma once 

#include "minidb/catalog/schema.hpp"
#include "minidb/storage/table/tuple.hpp"
#include "minidb/type/value.hpp"

namespace minidb {

    class AbstractExpression {
        public: 
        virtual ~AbstractExpression() = default;

        [[nodiscard]] virtual Value Evaluate(const Tuple& tuple, const Schema& schema) const = 0;
    };
}