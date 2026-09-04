#pragma once 

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "minidb/common/config.hpp"
#include "minidb/storage/index/b_plus_tree_leaf_page.hpp"
#include "minidb/storage/page/page.hpp"

namespace minidb {
    template <ByteCopyable K, typename Cmp>
    class BPlusTreeInternalPage {
        public:
        explicit BPlusTreeInternalPage(std::byte* data) noexcept : data_(data) {}

        void Init(int max_size) noexcept {
            WriteI32(kPageTypeOff, kInternalType);
            WriteI32(kSizeOff, 0);
            WriteI32(kMaxSizeOff, max_size);
        }

        [[nodiscard]] int GetSize() const noexcept { return ReadI32(kSizeOff); }
        [[nodiscard]] int GetMaxSize() const noexcept { return ReadI32(kMaxSizeOff); }
        void SetSize(int n) noexcept { WriteI32(kSizeOff, n); }

        [[nodiscard]] K KeyAt(int i) const noexcept { return Read<K>(EntryOff(i)); }
        void SetKeyAt(int i, const K& k) noexcept { Write<K>(EntryOff(i), k); }
        [[nodiscard]] page_id_t ValueAt(int i) const noexcept {
            return Read<page_id_t>(EntryOff(i) + sizeof(K));
        }

        void SetValueAt(int i, page_id_t v) noexcept { Write<page_id_t>(EntryOff(i) + sizeof(K), v); }
        
        [[nodiscard]] page_id_t LookUp(const K& key, const Cmp& cmp) const noexcept {
            int lo = 1;
            int hi = GetSize();
            while (lo < hi) {
                const int mid = lo + (hi - lo) / 2;
                if (cmp(KeyAt(mid), key) <= 0) {
                    lo = mid + 1;
                } else {
                hi = mid;
                }
            }
            return ValueAt(lo - 1);
        }
        //Acabou de ser criada não tem split
        void PopulateNewRoot(page_id_t old_child, const K& key, page_id_t new_child) noexcept {
            SetValueAt(0, old_child);
            SetKeyAt(1, key);
            SetValueAt(1, new_child);
            SetSize(2);
        }

        //calcula divisao de uma split
        void InsertAfter(page_id_t old_child, const K& key, page_id_t new_child) noexcept {
            const int idx = ValueIndex(old_child);
            const int pos = idx + 1;
            const int n = GetSize();
            for (int j = n; j > pos; --j) {
                SetKeyAt(j, KeyAt(j - 1));
                SetValueAt(j, ValueAt(j - 1));
            }
            SetKeyAt(pos, key);
            SetValueAt(pos, new_child);
            SetSize(n + 1);
        }

        void MoveHalfTo(BPlusTreeInternalPage& other) noexcept {
            const int n = GetSize();
            const int half = n / 2;
            int idx = 0;
            for (int j = half; j < n; ++j) {
                other.SetKeyAt(idx, KeyAt(j));
                other.SetValueAt(idx, ValueAt(j));
                ++idx;
            }
            other.SetSize(n - half);
            SetSize(half);
        }

        [[nodiscard]] int LookupIndex(const K& key, const Cmp& cmp) const noexcept {
            int lo = 1, hi = GetSize();
            while (lo < hi) {
                const int mid = lo + (hi - lo) / 2;
                if (cmp(KeyAt(mid), key) <= 0) {
                    lo = mid + 1;          
                } else {
                    { hi = mid;}
                }
            }
            return lo - 1;
        }

    void RemoveAt(int i) noexcept {
        const int n = GetSize();
        for (int j = i; j < n - 1; ++j) {
            SetKeyAt(j, KeyAt(j + 1)); SetValueAt(j, ValueAt(j + 1));
        }
        SetSize(n - 1);
    }
    // Left to right brother 
    K MoveLastToFrontOf(BPlusTreeInternalPage& recipient, const K& middle) noexcept {
        const int rn = recipient.GetSize();
        for (int j = rn; j > 0; --j) {
            recipient.SetKeyAt(j, recipient.KeyAt(j - 1));
            recipient.SetValueAt(j, recipient.ValueAt(j - 1));
        }
        recipient.SetValueAt(0, ValueAt(GetSize() - 1));
        recipient.SetKeyAt(1, middle);
        recipient.SetSize(rn + 1);
        const K new_middle = KeyAt(GetSize() - 1);
        SetSize(GetSize() - 1);
        return new_middle;
    }
    // Right to left brother
    K MoveFirstToEndOf(BPlusTreeInternalPage& recipient, const K& middle) noexcept {
        const int rn = recipient.GetSize();
        recipient.SetKeyAt(rn, middle);
        recipient.SetValueAt(rn, ValueAt(0));
        recipient.SetSize(rn +1 );
        const K new_middle = KeyAt(1);
        const int n = GetSize();
        for (int j = 0; j < n - 1; j++){
            SetKeyAt(j, KeyAt(j + 1));
            SetValueAt(j, ValueAt(j + 1));
        }
        SetSize(n - 1);
        return new_middle;
    }

    void MoveAllTo(BPlusTreeInternalPage& recipient, const K& middle) noexcept {
        const int rn = recipient.GetSize();
        const int n  = GetSize();
        recipient.SetKeyAt(rn, middle);
        recipient.SetValueAt(rn, ValueAt(0));
        for (int j = 1; j < n; ++j) {
            recipient.SetKeyAt(rn + j, KeyAt(j)); recipient.SetValueAt(rn + j, ValueAt(j));
        }
        recipient.SetSize(rn + n);
        SetSize(0);
    }

        private:
        [[nodiscard]] int ValueIndex(page_id_t v) const noexcept {
            const int n = GetSize();
            for (int i = 0; i < n; ++i) {
                if (ValueAt(i) == v) {
                    return i;
                }
            }
            return -1;
        }

        static constexpr std::size_t kPageTypeOff = 0;
        static constexpr std::size_t kSizeOff = 4;
        static constexpr std::size_t kMaxSizeOff = 8;
        static constexpr std::size_t kHeaderSize = 16;
        static constexpr std::size_t kInternalType = 0;

        [[nodiscard]] static std::size_t EntryOff(int i) noexcept {
            return kHeaderSize + static_cast<std::size_t>(i) * (sizeof(K) + sizeof(page_id_t));
        }
        template <typename T>
        [[nodiscard]] T Read(std::size_t off) const noexcept {
            T v;
            std::memcpy(&v, data_ + off, sizeof(T));
            return v;
        }
        template <typename T>
        void Write(std::size_t off, const T& v) noexcept { std::memcpy(data_ + off, &v, sizeof(T)); }
        [[nodiscard]] std::int32_t ReadI32(std::size_t off) const noexcept {
            return Read<std::int32_t>(off);
        }
        void WriteI32(std::size_t off, std::int32_t v) noexcept { Write<std::int32_t>(off, v); }

        std::byte* data_;
    };
}