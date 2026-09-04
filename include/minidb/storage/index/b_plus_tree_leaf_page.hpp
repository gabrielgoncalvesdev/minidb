#pragma once 

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

#include "minidb/common/config.hpp"

namespace minidb {
template <typename T>
concept ByteCopyable = std::is_trivially_copyable_v<T>;

template <ByteCopyable K, ByteCopyable V, typename Cmp>
class BPlusTreeLeafPage {
    public: 
    explicit BPlusTreeLeafPage(std::byte* data) noexcept : data_(data) {}

    void Init(int max_size) noexcept {
        WriteI32(kPageTypeOff, kLeafType);
        WriteI32(kSizeOff, 0);
        WriteI32(kMaxSizeOff, max_size);
        WriteI32(kNextOff, INVALID_PAGE_ID);
    }

    [[nodiscard]] int GetSize() const noexcept { return ReadI32(kSizeOff); }
    [[nodiscard]] int GetMaxSize() const noexcept { return ReadI32(kMaxSizeOff); }
    void SetSize(int n) noexcept { WriteI32(kSizeOff, n); }

    [[nodiscard]] page_id_t GetNextPageId() const noexcept { return ReadI32(kNextOff); }
    void SetNextPageId(page_id_t id) noexcept { WriteI32(kNextOff, id); }

    [[nodiscard]] K KeyAt(int i) const noexcept { return Read<K>(EntryOff(i)); }
    [[nodiscard]] V ValueAt(int i ) const noexcept { return Read<V>(EntryOff(i) + sizeof(K)); }

    [[nodiscard]] int KeyIndex(const K& key, const Cmp& cmp) const noexcept {
        int lo = 0;
        int hi = GetSize();
        while (lo < hi) {
            const int mid = lo + ( hi - lo ) / 2;
            if(cmp(KeyAt(mid), key) < 0) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo;
    }

    [[nodiscard]] std::optional<V> Lookup(const K& key, const Cmp& cmp) const noexcept {
        const int i = KeyIndex(key, cmp);
        if (i < GetSize() && cmp(KeyAt(i), key) == 0) {
            return ValueAt(i);
        }
        return std::nullopt;
    }

    bool Insert(const K& key, const V& value, const Cmp& cmp) noexcept {
        const int i = KeyIndex(key, cmp);
        if (i < GetSize() && cmp(KeyAt(i), key) == 0) {
            return false; 
        }
        const int n = GetSize();
        for (int j = n; j > i; --j) {
           SetEntry(j, KeyAt(j - 1), ValueAt(j - 1));
        }
        SetEntry(i, key, value);
        SetSize(n + 1);
        return true;
    }

    void MoveHalfTo(BPlusTreeLeafPage& other) noexcept {
        const int n = GetSize();
        const int half = n / 2;
        int idx = 0;
        for (int j = half; j < n; ++j) {
            other.SetEntry(idx, KeyAt(j), ValueAt(j));
            idx++;
        }
        other.SetSize(n - half);
        SetSize(half);

    }

    // remove begin.

    bool RemoveKey(const K& key, const Cmp& cmp) noexcept {
        const int i = KeyIndex(key, cmp);
        if (i >= GetSize() || cmp(KeyAt(i), key) != 0) { return false; }
        const int n = GetSize();
        for (int j = i; j < n - 1; ++j) {
            { SetEntry(j, KeyAt(j + 1), ValueAt(j + 1)); }
        }
        SetSize(n - 1);
        return true;
    }

    //two sister in minimum size 
    void MoveLastToFrontOf(BPlusTreeLeafPage& recipient) noexcept {
        const int n = GetSize();
        const K k = KeyAt(n - 1);
        const V v = ValueAt(n - 1);
        const int rn = recipient.GetSize();
        for (int j = rn; j > 0; --j) {
            recipient.SetEntry(j, recipient.KeyAt(j -1 ), recipient.ValueAt(j - 1));
        }
        recipient.SetEntry(0, k, v);
        recipient.SetSize(rn + 1);
        SetSize(n - 1);
    }

    void MoveFirstToEndOf(BPlusTreeLeafPage& recipient) noexcept {
        const K k = KeyAt(0);
        const V v = ValueAt(0);
        const int rn = recipient.GetSize();
        recipient.SetEntry(rn, k, v);
        recipient.SetSize(rn + 1);
        const int n = GetSize();
        for (int j = 0; j < n - 1; ++j) {
            SetEntry(j, KeyAt(j + 1), ValueAt(j + 1));
        }
        SetSize(n - 1);
    }

    void MoveAllTo(BPlusTreeLeafPage& recipient) noexcept {
        const int rn = recipient.GetSize();
        const int n = GetSize();
        for (int j = 0; j < n; ++j) {
            recipient.SetEntry(rn + j, KeyAt(j), ValueAt(j));
        }
        recipient.SetSize(rn + n);
        SetSize(0);
    }

    private:
    static constexpr std::size_t kPageTypeOff = 0;
    static constexpr std::size_t kSizeOff = 4;
    static constexpr std::size_t kMaxSizeOff = 8;
    static constexpr std::size_t kNextOff = 12;
    static constexpr std::size_t kHeaderSize = 16;
    static constexpr std::int32_t kLeafType = 1;

    [[nodiscard]] static std::size_t EntryOff(int i) noexcept {
        return kHeaderSize + static_cast<std::size_t>(i) * (sizeof(K) + sizeof(V));
    }
    void SetEntry(int i, const K& k, const V& v) noexcept {
        Write<K>(EntryOff(i), k);
        Write<V>(EntryOff(i) + sizeof(K), v);
    }

    template <typename T>
    [[nodiscard]] T Read(std::size_t off) const noexcept {
        T v;
        std::memcpy(&v, data_ + off, sizeof(T));
        return v;
    }
    template <typename T>
    void Write(std::size_t off, const T& v) noexcept {
        std::memcpy(data_ + off, &v, sizeof(T));
    }
    [[nodiscard]] std::int32_t ReadI32(std::size_t off) const noexcept {
        return Read<std::int32_t>(off);
    }
    void WriteI32(std::size_t off, std::int32_t v) noexcept { Write<std::int32_t>(off, v); }

    std::byte* data_;
};

}

