#pragma once 

#include <cstdint>
#include <vector>

#include "minidb/catalog/column.hpp"

namespace minidb {

    class Schema {
        public:
        explicit Schema(std::vector<Column> columns);

        [[nodiscard]] const std::vector<Column>& GetColumns() const noexcept { return columns_; }
        [[nodiscard]] const Column& GetColumn(std::uint32_t idx) const;
        [[nodiscard]] std::uint32_t GetColumnCount() const noexcept;

        [[nodiscard]] std::uint32_t InlinedSize() const noexcept { return fixed_length_; }

        [[nodiscard]] const std::vector<std::uint32_t>& UninlinedColumns() const noexcept {
            return uninlined_;
        } 

        private:
        std::vector<Column> columns_;
        std::uint32_t fixed_length_;
        std::vector<std::uint32_t> uninlined_;
    };
}