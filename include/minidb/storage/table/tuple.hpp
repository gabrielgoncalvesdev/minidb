#pragma once 

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "minidb/catalog/schema.hpp"
#include "minidb/common/rid.hpp"
#include "minidb/type/value.hpp"

namespace minidb {
    
    class Tuple {
        public:
        Tuple() = default;
        
        Tuple(const std::vector<Value>& values, const Schema& schema);
        Tuple(std::span<const std::byte> data, RID rid);

        [[nodiscard]] Value GetValue(const Schema& schema, std::uint32_t col_idx) const;

        [[nodiscard]] std::span<const std::byte> Data() const noexcept { return data_; }
        [[nodiscard]] std::uint32_t Size() const noexcept {
            return static_cast<std::uint32_t>(data_.size());
        }

        [[nodiscard]] const RID& GetRid() const noexcept { return rid_; }
        void SetRid(RID rid) noexcept { rid_ = rid; }

        private:
        RID rid_{};
        std::vector<std::byte> data_;
    };
}