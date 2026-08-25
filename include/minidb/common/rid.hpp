#pragma once 

#include "minidb/common/config.hpp"

namespace minidb {
    
    struct RID {
        page_id_t page_id = INVALID_PAGE_ID;
        slot_id_t slot_num = 0;

        [[nodiscard]] bool operator==(const RID&) const = default;
    };
}