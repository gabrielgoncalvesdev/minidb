#pragma once 

#include <vector>

#include "minidb/common/rid.hpp"
#include "minidb/execution/abstract_executor.hpp"
#include "minidb/storage/table/tuple.hpp"

namespace minidb {

    class ExecutionEngine {
        public:
        [[nodiscard]] static std::vector<Tuple> Execute(AbstractExecutor& executor) {
            std::vector<Tuple> results;
            executor.Init();
            Tuple tuple;
            RID rid;
            while (executor.Next(&tuple, &rid)) {
                results.push_back(tuple);
            }
            return results;
        }
    };
}