// include/minidb/common/version.hpp
#pragma once

#include <string_view>

namespace minidb {

[[nodiscard]] std::string_view Version() noexcept;

}  // namespace minidb
