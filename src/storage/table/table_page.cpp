#include "minidb/storage/table/table_page.hpp"
#include "minidb/common/config.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>

namespace minidb {
    namespace {
        constexpr std::size_t kHeaderSize = 12;
        constexpr std::size_t kSlotSize = 4;
        constexpr std::size_t kPageIdOffset = 0;
        constexpr std::size_t kNextPageIdOffset = 4;
        constexpr std::size_t kNumSlotsOffset = 8;
        constexpr std::size_t kFreeSpaceOffset = 10;

        std::uint16_t ReadU16(const std::byte* p) {
            std::uint16_t v;
            std::memcpy(&v, p, sizeof(v));
            return v;
        }
        void WriteU16(std::byte* p, std::uint16_t v) { std::memcpy(p, &v, sizeof(v) ); }
        
        std::int32_t ReadI32(const std::byte* p) {
            std::int32_t v;
            std::memcpy(&v, p, sizeof(v));
            return v;
        }
        void WriteI32(std::byte* p, std::int32_t v) {
            std::memcpy(p, &v, sizeof(v));
        }

        std::size_t SlotOffSet(std::uint16_t slot) {
            return kHeaderSize + static_cast<std::size_t>(slot) * kSlotSize;
        }
    }

    void TablePage::Init(page_id_t page_id) noexcept {
        WriteI32(data_ + kPageIdOffset, page_id);
        WriteI32(data_ + kNextPageIdOffset, INVALID_PAGE_ID);
        WriteU16(data_ + kNumSlotsOffset, 0);
        WriteU16(data_ + kFreeSpaceOffset, static_cast<std::uint16_t>(PAGE_SIZE));
    }

    page_id_t TablePage::GetPageId() const noexcept { return ReadI32(data_ + kPageIdOffset); }
    page_id_t TablePage::GetNextPageId() const noexcept { return ReadI32(data_ + kNextPageIdOffset); }
    void TablePage::SetNextPageId(page_id_t next) noexcept { WriteI32(data_ + kNextPageIdOffset, next); }
    std::uint16_t TablePage::GetNumTuples() const noexcept { return ReadU16(data_ + kNumSlotsOffset); }


    std::optional<std::uint16_t>  TablePage::InsertTuple(const Tuple& tuple) {
        const std::size_t num_slots = ReadU16(data_ + kNumSlotsOffset); // no offset 8 representado por um numero de 16 bits
        const std::size_t free_ptr = ReadU16(data_ + kFreeSpaceOffset); //começa 4096 bytes no offset 10 até 12
        const std::size_t size = tuple.Size(); //tamanho em bytes da tupla que tentarem inserir

        const std::size_t free_space =  free_ptr - SlotOffSet(static_cast<std::uint16_t>(num_slots)) ;

        if (free_space < size + kSlotSize) {
            return std::nullopt;
        }

        const std::size_t new_offset = free_ptr - size;
        std::memcpy(data_ + new_offset, tuple.Data().data(), size);

        std::byte* slot = data_ + SlotOffSet(static_cast<std::uint16_t>(num_slots));
        WriteU16(slot, static_cast<std::uint16_t>(new_offset));
        WriteU16(slot + 2 , static_cast<std::uint16_t>(size));

        WriteU16(data_ + kFreeSpaceOffset, static_cast<std::uint16_t>(new_offset));
        WriteU16(data_ + kNumSlotsOffset, static_cast<std::uint16_t>(num_slots + 1));
        return static_cast<std::uint16_t>(num_slots);
    }

    std::optional<Tuple> TablePage::GetTuple(std::uint16_t slot) const {
        if (slot >= GetNumTuples()) {
            return std::nullopt;
        }
        const std::byte* s = data_ + SlotOffSet(slot);
        const std::uint16_t offset = ReadU16(s);
        const std::uint16_t size = ReadU16(s + 2);
        if (size == 0) {
            return std::nullopt;
        }
        const std::span<const std::byte> bytes{data_ + offset, size};
        const RID rid{.page_id = GetPageId(), .slot_num = slot};
        return Tuple(bytes, rid);
    }

    bool TablePage::MarkDelete(std::uint16_t slot) {
        if (slot >= GetNumTuples()) {
            return false;
        }
        std::byte* s = data_ + SlotOffSet(slot);
        WriteU16(s + 2, 0);
        return true;
    }
}

