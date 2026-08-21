// include/minidb/storage/disk/disk_manager.hpp
#pragma once

#include <cstddef>
#include <expected>
#include <span>
#include <string>

#include "minidb/common/config.hpp"
#include "minidb/common/status.hpp"
#include "minidb/storage/disk/file_handle.hpp"

namespace minidb {

class DiskManager {
 public:
  explicit DiskManager(const std::string& db_path);

  [[nodiscard]] std::expected<void, Status> WritePage(page_id_t page_id,
                                                       std::span<const std::byte> data);

  [[nodiscard]] std::expected<void, Status> ReadPage(page_id_t page_id,
                                                      std::span<std::byte> out) const;

  [[nodiscard]] page_id_t AllocatePage() noexcept;
  [[nodiscard]] page_id_t NumPages() const noexcept { return next_page_id_; }

  void Sync();

 private:
  FileHandle db_file_;
  page_id_t  next_page_id_ = 0;
};

}  // namespace minidb
