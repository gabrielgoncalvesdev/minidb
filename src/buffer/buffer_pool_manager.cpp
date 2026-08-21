#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/storage/page/page.hpp"
#include <optional>
#include "minidb/common/config.hpp"
#include <cstddef>

#include <span>

namespace minidb {

    BufferPoolManager::BufferPoolManager(
        std::size_t pool_size, 
        DiskManager* disk_manager, 
        std::size_t k 
    ) : pool_size_(pool_size),
        disk_manager_(disk_manager),
        frames_(pool_size),
        replacer_(pool_size, k) {
            for (std::size_t i = 0; i < pool_size_; ++i) {
                free_list_.push_back(static_cast<frame_id_t>(i));
            }
        }

    std::optional<frame_id_t> BufferPoolManager::AllocateFrame() {
        if (!free_list_.empty()) {
            const frame_id_t fid = free_list_.front();
            free_list_.pop_front();
            return fid;
        }
        const std::optional<frame_id_t> victim = replacer_.Evict();
        if (!victim.has_value()) {
            return std::nullopt; // pool cheio tudo pin pin pinpinripom 
        }
        const frame_id_t fid = *victim;
        Page& frame = frames_[static_cast<std::size_t>(fid)];
        if (frame.is_dirty_) {
            const std::span<const std::byte> data{frame.data_.data(), PAGE_SIZE};
            (void)disk_manager_->WritePage(frame.page_id_, data);
        }
        page_table_.erase(frame.page_id_);
        return fid;
    } 

    Page* BufferPoolManager::NewPage(page_id_t* out_page_id) {
         const std::optional<frame_id_t> fid_opt = AllocateFrame();
         if (!fid_opt.has_value()) {
            return nullptr;
         }

         const frame_id_t fid = *fid_opt; 
         const page_id_t new_id = disk_manager_->AllocatePage();

         Page& frame = frames_[static_cast<std::size_t>(fid)];
         frame.Reset(new_id);
         frame.pin_count_ = 1;

         page_table_[new_id] = fid;
         replacer_.RecordAccess(fid);
         replacer_.SetEvictable(fid, false);

         if (out_page_id != nullptr) {
            *out_page_id = new_id;
         }
         return &frame;
    }

    Page* BufferPoolManager::FetchPage(page_id_t page_id) {
        if (auto it = page_table_.find(page_id); it != page_table_.end()) {
            const frame_id_t fid = it->second;
            Page& frame = frames_[static_cast<std::size_t>(fid)];
            ++frame.pin_count_;
            replacer_.RecordAccess(fid);
            replacer_.SetEvictable(fid, false);
            return &frame;
        }

        const std::optional<frame_id_t> fid_opt = AllocateFrame();
        if (!fid_opt.has_value()) {
            return nullptr;
        }
        const frame_id_t fid = *fid_opt;
        Page& frame = frames_[static_cast<std::size_t>(fid)];
        frame.Reset(page_id);
        frame.pin_count_ = 1;

        const std::span<std::byte> buf{frame.data_.data(), PAGE_SIZE};
        (void)disk_manager_->ReadPage(page_id, buf); //erros de IO

        page_table_[page_id] = fid;
        replacer_.RecordAccess(fid);
        replacer_.SetEvictable(fid, false);
        return &frame;
    }

    bool BufferPoolManager::UnpinPage(page_id_t page_id, bool is_dirty) {
        auto it = page_table_.find(page_id);
        if (it == page_table_.end()) {
            return false;
        }
        Page& frame = frames_[static_cast<std::size_t>(it->second)];
        if (frame.pin_count_ <= 0){
            return false;
        }
        if (is_dirty){
            frame.is_dirty_ = true;
        }
        --frame.pin_count_;
        if (frame.pin_count_ == 0) {
            replacer_.SetEvictable(it->second, true);
        }
        return true;
    }

    bool BufferPoolManager::FlushPage(page_id_t page_id) {
        auto it = page_table_.find(page_id);
        if (it == page_table_.end()) {
            return false;
        }
        Page& frame = frames_[static_cast<std::size_t>(it->second)];
        const std::span<const std::byte> data{frame.data_.data(), PAGE_SIZE};
        (void)disk_manager_->WritePage(page_id, data);
        frame.is_dirty_ = false;
        return true;
    }

    void BufferPoolManager::FlushAllPages() {
        for (const auto& [page_id, fid] : page_table_) {
            Page& frame = frames_[static_cast<std::size_t>(fid)];
            const std::span<const std::byte> data{frame.data_.data(), PAGE_SIZE};
            (void)disk_manager_->WritePage(page_id, data);
            frame.is_dirty_ = false;
        }
    }

    bool BufferPoolManager::DeletePage(page_id_t page_id) {
        auto it = page_table_.find(page_id);
        if (it == page_table_.end()) {
            return true;
        }
        const frame_id_t fid = it->second;
        Page& frame = frames_[static_cast<std::size_t>(fid)];
        if (frame.pin_count_ > 0) {
            return false; // in using 
        }
        page_table_.erase(it);
        replacer_.Remove(fid);
        frame.Reset(INVALID_PAGE_ID);
        free_list_.push_back(fid);
        return true;
    }
}