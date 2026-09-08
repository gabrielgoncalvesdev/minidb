#pragma once 

#include "minidb/catalog/schema.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/storage/table/tuple.hpp"

namespace minidb {

    class AbstractExecutor {
        public:
        virtual ~AbstractExecutor() = default;

        virtual void Init() = 0;
        virtual bool Next(Tuple* out_tuple, RID* out_rid) = 0;
        [[nodiscard]] virtual const Schema& GetOutputSchema() const = 0;
    };
}