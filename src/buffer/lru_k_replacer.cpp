#include "minidb/buffer/lru_k_replacer.hpp"
#include "minidb/common/config.hpp"

#include <cassert>
#include <cstddef>
#include <optional>

namespace minidb {
    LRUKReplacer::LRUKReplacer(std::size_t num_frames, std::size_t k) 
        : replacer_size_(num_frames), k_(k) {
            assert(k > 0 && "k needs to be >= 1");
        }

        void LRUKReplacer::RecordAccess(frame_id_t frame_id) {
            assert(static_cast<std::size_t>(frame_id) < replacer_size_ && "frame_id out of range");
            Node& node = node_store_[frame_id];
            node.history.push_front(current_timestamp_);
            if (node.history.size() > k_) {
                node.history.pop_back();
            }
            ++current_timestamp_;
        }

        void LRUKReplacer::SetEvictable(frame_id_t frame_id, bool set_evictable) {
            assert(static_cast<std::size_t>(frame_id) < replacer_size_ && "frame_id out of range");
            auto it = node_store_.find(frame_id);
            if (it == node_store_.end()) {
                return;
            }
            Node& node = it->second;
            if (node.is_evictable == set_evictable) {
                return;
            }
            node.is_evictable = set_evictable;
            if (set_evictable) {
                ++curr_size_;
            } else {
                --curr_size_;
            }
        }

        void LRUKReplacer::Remove(frame_id_t frame_id) {
            auto it = node_store_.find(frame_id);
            if (it == node_store_.end()) {
                return;
            }
            if (it->second.is_evictable) {
                --curr_size_;
            }
            node_store_.erase(it);
        }

        std::optional<frame_id_t> LRUKReplacer::Evict() {
            bool found = false;
            frame_id_t victim = 0;
            bool victim_inf = false;
            std::size_t victim_key = 0;

            for (const auto& [fid, node] : node_store_) {
                if (!node.is_evictable) {
                    continue;
                }
                const bool is_inf = node.history.size() < k_;
                const std::size_t oldest = node.history.back();

                bool take = false;
                if (!found) {
                    take = true;
                } else if (is_inf && !victim_inf) {
                    take = true;
                } else if (is_inf == victim_inf && oldest < victim_key) {
                    take = true;
                }

                if (take) {
                    found = true;
                    victim = fid;
                    victim_inf = is_inf;
                    victim_key = oldest;
                }
            }

            if (!found) {
                return std::nullopt;
            }
            node_store_.erase(victim);
            --curr_size_;
            return victim;
        }
}