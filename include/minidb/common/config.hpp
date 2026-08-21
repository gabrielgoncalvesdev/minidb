// include/minidb/common/config.hpp
#pragma once

#include <cstddef>   // std::size_t
#include <cstdint>   // std::int32_t, std::int64_t, std::uint16_t

namespace minidb {

using page_id_t  = std::int32_t;
using frame_id_t = std::int32_t;
using txn_id_t   = std::int64_t;
using lsn_t      = std::int64_t;
using slot_id_t  = std::uint16_t;

inline constexpr std::size_t PAGE_SIZE        = 4096;
inline constexpr std::size_t BUFFER_POOL_SIZE = 128;

inline constexpr page_id_t INVALID_PAGE_ID = -1;
inline constexpr lsn_t     INVALID_LSN     = -1;
inline constexpr txn_id_t  INVALID_TXN_ID  = -1;

}  // namespace minidb
