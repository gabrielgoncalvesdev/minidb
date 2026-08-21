#pragma once 

#include <array>
#include <cstddef>

#include "minidb/common/config.hpp"

namespace minidb {

    class Page {
        public:
        Page() = default;

        [[nodiscard]] std::byte* Data() noexcept { return data_.data(); }
        [[nodiscard]] const std::byte* Data() const noexcept {return data_.data(); }

        [[nodiscard]] page_id_t PageId() const noexcept { return page_id_; }
        [[nodiscard]] int PinCount() const noexcept { return pin_count_;}
        [[nodiscard]] bool IsDirty() const noexcept { return is_dirty_; }

        private: 
            friend class BufferPoolManager;

            void Reset(page_id_t page_id) {
                data_.fill(std::byte{0});
                page_id_ = page_id;
                pin_count_ = 0;
                is_dirty_ = false;
            }

            std::array<std::byte, PAGE_SIZE> data_{};
            page_id_t page_id_ = INVALID_PAGE_ID;
            int pin_count_ = 0;
            bool is_dirty_ = false;
    };
}