#pragma once 

#include <cstddef>
#include <list> 
#include <optional>
#include <unordered_map>

#include "minidb/common/config.hpp"

namespace minidb {

    class LRUKReplacer {
        public:
        LRUKReplacer(std::size_t num_frames, std::size_t k);

        [[nodiscard]] std::optional<frame_id_t> Evict();

        void RecordAccess(frame_id_t frame_id);

        void SetEvictable(frame_id_t frame_id, bool set_evictable);

        void Remove(frame_id_t frame_id);

        [[nodiscard]] std::size_t Size() const noexcept { return curr_size_; }

        private: 
            struct Node {
                std::list<std::size_t> history;
                bool is_evictable = false;
            };

            std::unordered_map<frame_id_t, Node> node_store_;
            std::size_t current_timestamp_ = 0;
            std::size_t curr_size_ = 0;
            std::size_t replacer_size_; 
            std::size_t k_;
    };
}
