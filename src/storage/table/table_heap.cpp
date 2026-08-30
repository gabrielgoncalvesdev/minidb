#include "minidb/storage/table/table_heap.hpp"

#include <cassert>
#include <optional>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/common/config.hpp"
#include "minidb/storage/page/page.hpp"
#include "minidb/storage/page/page_guard.hpp"
#include "minidb/storage/table/table_page.hpp"

namespace minidb {
    TableHeap::TableHeap(BufferPoolManager* bpm) : bpm_(bpm) {
        page_id_t id = INVALID_PAGE_ID;
        Page* p = bpm_->NewPage(&id);
        assert(p != nullptr && "buffer pool has not space for the first page");
        PageGuard guard(bpm_, p);
        TablePage tp(guard.DataMut());
        tp.Init(id);
        first_page_id_ = id;
        last_page_id_ = id;
    }

    std::optional<RID> TableHeap::InsertTuple(const Tuple& tuple) {
        {
            Page* p = bpm_->FetchPage(last_page_id_);
            if (p == nullptr) {
                return std::nullopt;
            }
            PageGuard guard(bpm_, p);
            TablePage tp(guard.DataMut());
            if (auto slot = tp.InsertTuple(tuple)) {
                return RID{.page_id = last_page_id_, .slot_num = *slot};
            }
        }

        page_id_t new_id = INVALID_PAGE_ID;
        Page* np = bpm_->NewPage(&new_id);
        if (np == nullptr) {
            return std::nullopt;
        }
        PageGuard new_guard(bpm_, np);
        TablePage new_tp(new_guard.DataMut());
        new_tp.Init(new_id);
        {
            Page* old = bpm_->FetchPage(last_page_id_);
            if ( old == nullptr) {
                return std::nullopt;
            }
            PageGuard old_guard(bpm_, old);
            TablePage old_tp(old_guard.DataMut());

            old_tp.SetNextPageId(new_id);
        }
        last_page_id_ = new_id;

        auto slot = new_tp.InsertTuple(tuple);
        if (!slot) {
            return std::nullopt;
        }
        return RID{.page_id = new_id, .slot_num = *slot};
    }

    std::optional<Tuple> TableHeap::GetTuple(RID rid) const {
        Page* p = bpm_->FetchPage(rid.page_id);
        if (p == nullptr) {
            return std::nullopt;
        }
        PageGuard guard(bpm_, p);
        TablePage tp(p->Data());
        return tp.GetTuple(rid.slot_num);
    }

    bool TableHeap::MarkDelete(RID rid) {
        Page* p = bpm_->FetchPage(rid.page_id);
        if (p == nullptr) {
            return false;
        }
        PageGuard guard(bpm_, p);
        TablePage tp(guard.DataMut());
        return tp.MarkDelete(rid.slot_num);
    }

    TableHeap::Iterator::Iterator(BufferPoolManager* bpm, page_id_t page_id, std::uint16_t slot)
    : bpm_(bpm), page_id_(page_id), slot_(slot) {
        AdvanceToValid();
    }

    void TableHeap::Iterator::AdvanceToValid() {
        while (page_id_ != INVALID_PAGE_ID) {
            Page* p = bpm_->FetchPage(page_id_);
            if (p == nullptr) {
                page_id_ = INVALID_PAGE_ID;
                return;
            }
            PageGuard guard(bpm_, p);
            TablePage tp(p->Data());
            const std::uint16_t n = tp.GetNumTuples();
            while (slot_ < n) {
                if (tp.GetTuple(slot_).has_value()) {
                    return;
                }
                ++slot_;
            }
            page_id_ = tp.GetNextPageId();
            slot_ = 0;
        }
    }

    Tuple TableHeap::Iterator::operator*() const  {
        Page* p = bpm_->FetchPage(page_id_);
        PageGuard guard(bpm_, p);
        TablePage tp(p->Data());
        return tp.GetTuple(slot_).value();
    }

    TableHeap::Iterator& TableHeap::Iterator::operator++() {
        ++slot_;
        AdvanceToValid();
        return *this;
    }

    bool TableHeap::Iterator::operator==(const Iterator& other) const noexcept {
        return page_id_ == other.page_id_ && slot_ == other.slot_;
    }

    TableHeap::Iterator TableHeap::begin() { return Iterator(bpm_, first_page_id_, 0); }
    TableHeap::Iterator TableHeap::end() { return Iterator(bpm_, INVALID_PAGE_ID, 0); }
}
