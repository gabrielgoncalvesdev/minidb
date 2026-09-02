#pragma once

#include <utility>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/common/config.hpp"
#include "minidb/storage/page/page.hpp"

namespace minidb {

    class PageGuard {
        public:
        PageGuard() = default; 
        PageGuard(BufferPoolManager* bpm, Page* page) noexcept : bpm_(bpm), page_(page) {}

        ~PageGuard() { Drop(); }

        PageGuard(const PageGuard&) = delete;
        PageGuard& operator=(const PageGuard&) = delete;

        PageGuard(PageGuard&& other) noexcept
        : bpm_(std::exchange(other.bpm_, nullptr)),
        page_(std::exchange(other.page_, nullptr)),
        is_dirty_(std::exchange(other.is_dirty_, false)) {}

        PageGuard& operator=(PageGuard&& other) noexcept {
            if (this != &other) {
                Drop();
                bpm_ = std::exchange(other.bpm_, nullptr);
                page_ = std::exchange(other.page_, nullptr);
                is_dirty_ = std::exchange(other.is_dirty_, false);
            }
            return *this;
        }

        [[nodiscard]] bool IsValid() const noexcept { return page_ != nullptr; }
        [[nodiscard]] page_id_t PageId() const noexcept { return page_->PageId(); }

        [[nodiscard]] const std::byte* Data() const noexcept { return page_->Data(); }

        [[nodiscard]] std::byte* DataMut() noexcept {
            is_dirty_ = true;
            return page_->Data();
        }

        void Drop() noexcept {
            if (bpm_ != nullptr && page_ != nullptr) {
                bpm_->UnpinPage(page_->PageId(), is_dirty_);
            }
            bpm_ = nullptr;
            page_ = nullptr;
            is_dirty_ = false;
        }

        private:
        BufferPoolManager* bpm_ = nullptr;
        Page* page_ = nullptr;
        bool is_dirty_ = false;

    };
}