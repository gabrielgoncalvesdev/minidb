// src/storage/disk/disk_manager.cpp
#include "minidb/storage/disk/disk_manager.hpp"

#include <cassert>
#include <cstring>  // std::memset

namespace minidb {

namespace {
[[nodiscard]] std::size_t PageOffset(page_id_t id) noexcept {
  return static_cast<std::size_t>(id) * PAGE_SIZE;
}
}  // namespace

DiskManager::DiskManager(const std::string& db_path) : db_file_(db_path) {
  next_page_id_ = static_cast<page_id_t>(db_file_.SizeBytes() / PAGE_SIZE);
}

page_id_t DiskManager::AllocatePage() noexcept {
  return next_page_id_++;
}

std::expected<void, Status> DiskManager::WritePage(page_id_t page_id,
                                                   std::span<const std::byte> data) {
  assert(data.size() == PAGE_SIZE && "WritePage exige buffer de exatamente PAGE_SIZE");
  if (page_id < 0 || page_id >= next_page_id_) {
    return std::unexpected(Status::kPageOutOfRange);
  }
  if (!db_file_.WriteAt(PageOffset(page_id), data)) {
    return std::unexpected(Status::kIoError);
  }
  return {};
}

std::expected<void, Status> DiskManager::ReadPage(page_id_t page_id,
                                                  std::span<std::byte> out) const {
  assert(out.size() == PAGE_SIZE && "ReadPage exige buffer de exatamente PAGE_SIZE");
  if (page_id < 0 || page_id >= next_page_id_) {
    return std::unexpected(Status::kPageOutOfRange);
  }
  const std::size_t bytes_read = db_file_.ReadAt(PageOffset(page_id), out);
  if (bytes_read < out.size()) {
    std::memset(out.data() + bytes_read, 0, out.size() - bytes_read);
  }
  return {};
}

void DiskManager::Sync() { db_file_.Sync(); }

}  // namespace minidb
