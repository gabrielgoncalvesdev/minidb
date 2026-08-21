#pragma once 

#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

#include "minidb/buffer/lru_k_replacer.hpp"
#include "minidb/common/config.hpp"
#include "minidb/storage/disk/disk_manager.hpp"
#include "minidb/storage/page/page.hpp"

namespace minidb {
    class BufferPoolManager {
        public:
        BufferPoolManager(std::size_t pool_size, DiskManager* disk_manager, std::size_t k = 2);

        [[nodiscard]] std::size_t PoolSize() const noexcept { return pool_size_; }

        [[nodiscard]] Page* NewPage(page_id_t* out_page_id);

        [[nodiscard]] Page* FetchPage(page_id_t page_id); // devolve pagina fixada 

        bool UnpinPage(page_id_t page_id, bool is_dirty); // reduz contagem pin 

        bool FlushPage(page_id_t page_id); // grava pagina no disco 
        void FlushAllPages();

        bool DeletePage(page_id_t page_id); // remove pagina do pool

        private: 
        
        [[nodiscard]] std::optional<frame_id_t> AllocateFrame();

        std::size_t pool_size_;
        DiskManager* disk_manager_; //ponteiro / nÃo dono 
        std::vector<Page> frames_;
        std::unordered_map<page_id_t, frame_id_t> page_table_;
        std::list<frame_id_t> free_list_; // liste por causa de problema de O(n) 
        LRUKReplacer replacer_;
    };
}