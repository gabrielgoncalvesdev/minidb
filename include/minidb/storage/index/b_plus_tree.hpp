#pragma once 

#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

#include "minidb/buffer/buffer_pool_manager.hpp"
#include "minidb/common/config.hpp"
#include "minidb/storage/index/b_plus_tree_internal_page.hpp"
#include "minidb/storage/index/b_plus_tree_leaf_page.hpp"
#include "minidb/storage/page/page.hpp"
#include "minidb/storage/page/page_guard.hpp"

namespace minidb {
    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    class BPlusTree {
        public:
        BPlusTree(BufferPoolManager* bpm, Cmp cmp, int leaf_max_size, int internal_max_size)
        : bpm_(bpm),
        cmp_(cmp),
        leaf_max_size_(leaf_max_size),
        internal_max_size_(internal_max_size) {}


        [[nodiscard]] bool IsEmpty() const noexcept {return root_page_id_ == INVALID_PAGE_ID; }

        bool Insert(const K& key, const V& value);
        [[nodiscard]] std::optional<V> GetValue(const K& key) const;


        void Remove(const K& key);

        private:
        using LeafPage = BPlusTreeLeafPage<K, V, Cmp>;
        using InternalPage = BPlusTreeInternalPage<K, Cmp>;
        struct SplitInfo {
            K key;
            page_id_t page_id;
        };
        
        [[nodiscard]] int LeafMin() const noexcept { return leaf_max_size_ / 2; }
        [[nodiscard]] int InternalMin() const noexcept { return internal_max_size_ / 2; }
        void RemoveRec(page_id_t page_id, const K& key);
        void FixLeafUnderflow(InternalPage& node, int ci);
        void FixInternalUnderflow(InternalPage& node, int ci);


        [[nodiscard]] page_id_t LeftmostLeaf() const;

        [[nodiscard]] static bool IsLeaf(const std::byte* data) noexcept {
            std::int32_t type;
            std::memcpy(&type, data, sizeof(type));
            return type == 1;
        }

        std::optional<SplitInfo> InsertRec(page_id_t page_id, const K& key, const V& value, bool& dup);
        SplitInfo SplitLeaf(LeafPage& leaf);
        SplitInfo SplitInternal(InternalPage& node);

        BufferPoolManager* bpm_;
        Cmp cmp_;
        int leaf_max_size_;
        int internal_max_size_;
        page_id_t root_page_id_ = INVALID_PAGE_ID;

        public:
        class Iterator{ 
            public:
            Iterator(BufferPoolManager* bpm, page_id_t leaf_id, int index) noexcept 
            : bpm_(bpm), leaf_id_(leaf_id), index_(index) {}

            [[nodiscard]] bool IsEnd() const noexcept { return leaf_id_ == INVALID_PAGE_ID; }

            [[nodiscard]] std::pair<K, V> operator*() const {
                Page* p = bpm_->FetchPage(leaf_id_);
                PageGuard guard(bpm_, p);
                LeafPage leaf(p->Data());
                return {leaf.KeyAt(index_), leaf.ValueAt(index_)};
            }

            Iterator& operator++() {
                Page* p = bpm_->FetchPage(leaf_id_);
                PageGuard guard(bpm_, p);
                LeafPage leaf(p->Data());
                if ( index_ + 1  < leaf.GetSize()) {
                   ++index_;
                } else {
                    leaf_id_ = leaf.GetNextPageId();
                    index_ = 0;
                }

                return *this;
            }

            [[nodiscard]] bool operator==(const Iterator& o) const noexcept {
                return leaf_id_ == o.leaf_id_ && index_ == o.index_;
            }

            [[nodiscard]] bool operator!=(const Iterator& o) const noexcept {
                return !(*this == o);
            }

            private:
            BufferPoolManager* bpm_;
            page_id_t leaf_id_;
            int index_;
        };

        [[nodiscard]] Iterator End() const noexcept { return Iterator(bpm_, INVALID_PAGE_ID, 0); }
        [[nodiscard]] Iterator Begin() const {
            if (root_page_id_ == INVALID_PAGE_ID) {
                return End();
            }
            return Iterator(bpm_, LeftmostLeaf(), 0);
        }
    };

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    page_id_t BPlusTree<K, V, Cmp>::LeftmostLeaf() const {
    page_id_t cur = root_page_id_;
    while (true) {
        Page* p = bpm_->FetchPage(cur);
        PageGuard guard(bpm_, p);
        if (IsLeaf(p->Data())) {
        return cur;
        }
        InternalPage node(p->Data());
        cur = node.ValueAt(0);  // filho mais à esquerda
    }
    }

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    bool BPlusTree<K, V, Cmp>::Insert(const K& key, const V& value) {
        if (root_page_id_ == INVALID_PAGE_ID) {
            page_id_t root_id = INVALID_PAGE_ID;
            Page* p = bpm_->NewPage(&root_id);
            PageGuard guard(bpm_, p);
            LeafPage leaf(guard.DataMut());
            leaf.Init(leaf_max_size_);
            leaf.Insert(key, value, cmp_);
            root_page_id_ = root_id;
            return true;
        }
        bool dup = false;
        std::optional<SplitInfo> res = InsertRec(root_page_id_, key, value, dup);
        if (dup) {
            return false;
        }
        if (res.has_value()) {
            page_id_t new_root_id = INVALID_PAGE_ID;
            Page* p = bpm_->NewPage(&new_root_id);
            PageGuard guard(bpm_, p);
            InternalPage new_root(guard.DataMut());
            new_root.Init(internal_max_size_);
            new_root.PopulateNewRoot(root_page_id_, res->key, res->page_id);
            root_page_id_ = new_root_id;
        }
        return true;
    }

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    auto BPlusTree<K, V, Cmp>::InsertRec(page_id_t page_id, const K& key, const V& value, bool& dup)
    -> std::optional<SplitInfo> {
        Page* p = bpm_->FetchPage(page_id);
        PageGuard guard(bpm_, p);
        if (IsLeaf(p->Data())) {
            LeafPage leaf(guard.DataMut());
            if (!leaf.Insert(key, value, cmp_)) {
                dup = true;
                return std::nullopt;
            }
            if (leaf.GetSize() <= leaf_max_size_) {
                return std::nullopt;
            }
            return SplitLeaf(leaf);
        }

        InternalPage node(guard.DataMut());
        const page_id_t child = node.LookUp(key, cmp_);
        std::optional<SplitInfo> res = InsertRec(child, key, value, dup);
        if(!res.has_value()) {
            return std::nullopt;
        }
        node.InsertAfter(child, res->key, res->page_id);
        if (node.GetSize() <= internal_max_size_) {
            return std::nullopt;
        }
        return SplitInternal(node);
    }


    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    auto BPlusTree<K, V, Cmp>::SplitInternal(InternalPage& node) -> SplitInfo {
        page_id_t new_id = INVALID_PAGE_ID;
        Page* np = bpm_->NewPage(&new_id);
        PageGuard guard(bpm_, np);
        InternalPage new_node(guard.DataMut());
        new_node.Init(internal_max_size_);
        const int half = node.GetSize() / 2;
        const K push_key = node.KeyAt(half);
        node.MoveHalfTo(new_node);
        return SplitInfo(push_key, new_id);
    }

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    auto BPlusTree<K, V, Cmp>::SplitLeaf(LeafPage& leaf) -> SplitInfo {
        page_id_t new_id = INVALID_PAGE_ID;
        Page* np = bpm_->NewPage(&new_id);
        PageGuard guard(bpm_, np);
        LeafPage new_leaf(guard.DataMut());
        new_leaf.Init(leaf_max_size_);
        //const int half = leaf.GetSize() / 2;
        leaf.MoveHalfTo(new_leaf);
        new_leaf.SetNextPageId(leaf.GetNextPageId());
        leaf.SetNextPageId(new_id);
        const K push_key = new_leaf.KeyAt(0);
        return SplitInfo(push_key, new_id);
    }

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    std::optional<V> BPlusTree<K, V, Cmp>::GetValue(const K& key) const {
        if (root_page_id_ == INVALID_PAGE_ID) {
            return std::nullopt;
        }
        page_id_t cur = root_page_id_;
        while (true) {
        Page* p = bpm_->FetchPage(cur);
        PageGuard guard(bpm_, p);
        if (IsLeaf(p->Data())) {
            LeafPage leaf(p->Data());
            return leaf.Lookup(key, cmp_);
        }
        InternalPage node(p->Data());
        cur = node.LookUp(key, cmp_);
        }
    };

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    void BPlusTree<K, V, Cmp>::Remove(const K& key) {
    if (root_page_id_ == INVALID_PAGE_ID) { return; }
    RemoveRec(root_page_id_, key);
    Page* p = bpm_->FetchPage(root_page_id_);
    PageGuard guard(bpm_, p);
    if (IsLeaf(p->Data())) {
        LeafPage leaf(p->Data());
        if (leaf.GetSize() == 0) {  
        const page_id_t old = root_page_id_;
        root_page_id_ = INVALID_PAGE_ID;
        guard.Drop();
        bpm_->DeletePage(old);
        }
    } else {
        InternalPage node(p->Data());
        if (node.GetSize() == 1) {  
        const page_id_t old = root_page_id_;
        const page_id_t new_root = node.ValueAt(0);
        root_page_id_ = new_root;
        guard.Drop();
        bpm_->DeletePage(old);
        }
    }
    };

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    void BPlusTree<K, V, Cmp>::RemoveRec(page_id_t page_id, const K& key) {
        Page* p = bpm_->FetchPage(page_id);
        PageGuard guard(bpm_, p);
        if (IsLeaf(p->Data())) {
            LeafPage leaf(guard.DataMut());
            leaf.RemoveKey(key, cmp_);
            return;
        }
        InternalPage node(guard.DataMut());
        const int ci = node.LookupIndex(key, cmp_);
        const page_id_t child_id = node.ValueAt(ci);
        RemoveRec(child_id, key);

        bool child_is_leaf = false;
        int child_size = 0;
        {
            Page* cp = bpm_->FetchPage(child_id);
            PageGuard cg(bpm_, cp);
            child_is_leaf = IsLeaf(cp->Data());
            child_size = child_is_leaf ? LeafPage(cp->Data()).GetSize() : InternalPage(cp->Data()).GetSize();
        }
        if (child_is_leaf) {
            if (child_size < LeafMin()) {
                FixLeafUnderflow(node, ci);
            }
        } else {
            if (child_size < InternalMin()) {
                FixInternalUnderflow(node, ci);
            }
        }
    };

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    void BPlusTree<K, V, Cmp>::FixInternalUnderflow(InternalPage& node, int ci) {
    const page_id_t child_id = node.ValueAt(ci);
    Page* cp = bpm_->FetchPage(child_id);
    PageGuard cg(bpm_, cp);
    InternalPage child(cg.DataMut());
    if (ci > 0) {  // irmão à ESQUERDA (índice ci-1)
        const page_id_t left_id = node.ValueAt(ci - 1);
        Page* lp = bpm_->FetchPage(left_id);
        PageGuard lg(bpm_, lp);
        InternalPage left(lg.DataMut());
        if (left.GetSize() > InternalMin()) {              // EMPRESTA do esquerdo
        const K new_mid = left.MoveLastToFrontOf(child, node.KeyAt(ci));
        node.SetKeyAt(ci, new_mid);
        } else {                                           // FUNDE o filho no esquerdo
        child.MoveAllTo(left, node.KeyAt(ci));           // puxa o separador do pai pra baixo
        node.RemoveAt(ci);
        cg.Drop();
        bpm_->DeletePage(child_id);
        }
    } else {  // irmão à DIREITA (índice ci+1)
        const page_id_t right_id = node.ValueAt(ci + 1);
        Page* rp = bpm_->FetchPage(right_id);
        PageGuard rg(bpm_, rp);
        InternalPage right(rg.DataMut());
        if (right.GetSize() > InternalMin()) {             // EMPRESTA do direito
        const K new_mid = right.MoveFirstToEndOf(child, node.KeyAt(ci + 1));
        node.SetKeyAt(ci + 1, new_mid);
        } else {                                           // FUNDE o direito no filho
        right.MoveAllTo(child, node.KeyAt(ci + 1));
        node.RemoveAt(ci + 1);
        rg.Drop();
        bpm_->DeletePage(right_id);
        }
    }
    };

    template <ByteCopyable K, ByteCopyable V, typename Cmp>
    void BPlusTree<K, V, Cmp>::FixLeafUnderflow(InternalPage& node, int ci) {
        const page_id_t child_id = node.ValueAt(ci);
        Page* cp = bpm_->FetchPage(child_id);
        PageGuard cg(bpm_, cp);
        LeafPage child(cg.DataMut());
        if (ci > 0) {
            const page_id_t left_id = node.ValueAt(ci - 1);
            Page* p = bpm_->FetchPage(left_id);
            PageGuard pg(bpm_, p);
            LeafPage left(pg.DataMut());
            if (left.GetSize() > LeafMin()) {
                left.MoveLastToFrontOf(child);
                node.SetKeyAt(ci, child.KeyAt(0));
            } else {
                child.MoveAllTo(left);
                left.SetNextPageId(child.GetNextPageId());
                node.RemoveAt(ci);
                cg.Drop();
                bpm_->DeletePage(child_id);
            }
            } else  {
            const page_id_t right_id = node.ValueAt(ci + 1);
           Page* rp = bpm_->FetchPage(right_id);
           PageGuard rg(bpm_, rp);
           LeafPage right(rg.DataMut());
           if (right.GetSize() > LeafMin()) {
            right.MoveFirstToEndOf(child);
            node.SetKeyAt(ci + 1, right.KeyAt(0));
           } else {
            right.MoveAllTo(child);
            child.SetNextPageId(right.GetNextPageId());
            node.RemoveAt(ci + 1);
            rg.Drop();
            bpm_->DeletePage(right_id);
            }
        }
    }  
}