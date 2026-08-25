#include "minidb/catalog/schema.hpp"

#include <cassert>
#include <vector>

namespace minidb {
    Schema::Schema(std::vector<Column> columns) : columns_(std::move(columns)) {
        std::uint32_t offset = 0;
        for (std::uint32_t i = 0; i < columns_.size(); ++i) {
            columns_[i].offset_ = offset;
            if (!columns_[i].IsInlined()) {
                uninlined_.push_back(i);
            }
            offset += columns_[i].FixedSize();
        }
        fixed_length_ = offset;
    }

    const Column& Schema::GetColumn(std::uint32_t idx) const {
        assert(idx < columns_.size());
        return columns_[idx];
    }

    std::uint32_t Schema::GetColumnCount() const noexcept {
        return static_cast<std::uint32_t>(columns_.size());
    }
}