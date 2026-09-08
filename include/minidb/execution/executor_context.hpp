#pragma once 

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/catalog/catalog.hpp"

namespace minidb {

    class ExecutorContext {
        public:
        ExecutorContext(Catalog* catalog, BufferPoolManager* bpm) noexcept : catalog_(catalog), bpm_(bpm) {}

        [[nodiscard]] Catalog* GetCatalog() const noexcept { return catalog_; }
        [[nodiscard]] BufferPoolManager* GetBufferPool() const noexcept { return bpm_; }

        private:
        Catalog* catalog_;
        BufferPoolManager* bpm_;
    };
}