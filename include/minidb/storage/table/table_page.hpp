#pragma once 

#include <cstddef>
#include <cstdint>
#include <optional>

#include "minidb/common/config.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/storage/page/page.hpp"
#include "minidb/storage/table/tuple.hpp"

namespace minidb {

    class TablePage {
        public: 
        explicit TablePage(std::byte* page_data) noexcept : data_(page_data) {}

        void Init(page_id_t page_id) noexcept;

        [[nodiscard]] page_id_t GetPageId() const noexcept;
        [[nodiscard]] page_id_t GetNextPageId() const noexcept;
        void SetNextPageId(page_id_t next) noexcept;
        [[nodiscard]] std::uint16_t GetNumTuples() const noexcept;


        [[nodiscard]] std::optional<std::uint16_t> InsertTuple(const Tuple& tuple);
        [[nodiscard]] std::optional<Tuple> GetTuple(std::uint16_t slot) const;
        bool MarkDelete(std::uint16_t slot); 

        private:
        std::byte* data_;
    };
}