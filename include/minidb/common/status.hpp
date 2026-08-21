// include/minidb/common/status.hpp
#pragma once

#include <string_view>

namespace minidb {

enum class Status {
  kIoError,
  kPageOutOfRange,
  kOutOfSpace,
  kNotFound,
  kCorrupted,
};

[[nodiscard]] constexpr std::string_view ToString(Status s) noexcept {
  switch (s) {
    case Status::kIoError:        return "IoError";
    case Status::kPageOutOfRange: return "PageOutOfRange";
    case Status::kOutOfSpace:     return "OutOfSpace";
    case Status::kNotFound:       return "NotFound";
    case Status::kCorrupted:      return "Corrupted";
  }
  return "Unknown";
}

}  // namespace minidb
