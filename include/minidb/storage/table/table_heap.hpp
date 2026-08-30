#pragma once 

#include <cstdint>
#include <optional>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/common/config.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/storage/page/page.hpp"
#include "minidb/storage/table/table_page.hpp"
#include "minidb/storage/table/tuple.hpp"

namespace minidb {

    class TableHeap{
        public:
        explicit TableHeap(BufferPoolManager* bpm);

        [[nodiscard]] std::optional<RID> InsertTuple(const Tuple& tuple);
        [[nodiscard]] std::optional<Tuple> GetTuple(RID rid) const;
        bool MarkDelete(RID rid);

        [[nodiscard]] page_id_t FirstPageId() const noexcept { return first_page_id_; }
        
        class Iterator {
            public:
            Iterator(BufferPoolManager* bpm, page_id_t page_id, std::uint16_t slot);

            [[nodiscard]] Tuple operator*() const;
            Iterator& operator++();
            [[nodiscard]] bool operator==(const Iterator& other) const noexcept;
            [[nodiscard]] bool operator!=(const Iterator& other) const noexcept{ 
                return !(*this == other);
            }

            private:
        void AdvanceToValid(); 

        BufferPoolManager* bpm_;
        page_id_t page_id_;
        std::uint16_t slot_;
        };

        [[nodiscard]] Iterator begin();
        [[nodiscard]] Iterator end();

        private:
        BufferPoolManager* bpm_;
        page_id_t first_page_id_ = INVALID_PAGE_ID;
        page_id_t last_page_id_ = INVALID_PAGE_ID;
    };
}

