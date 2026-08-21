// include/minidb/storage/disk/file_handle.hpp
#pragma once

#include <cstddef>
#include <span>
#include <string>

namespace minidb {

class FileHandle {
 public:
  explicit FileHandle(const std::string& path);
  ~FileHandle();

  FileHandle(const FileHandle&)            = delete;
  FileHandle& operator=(const FileHandle&) = delete;

  FileHandle(FileHandle&& other) noexcept;
  FileHandle& operator=(FileHandle&& other) noexcept;

  [[nodiscard]] bool WriteAt(std::size_t offset, std::span<const std::byte> data);
  [[nodiscard]] std::size_t ReadAt(std::size_t offset, std::span<std::byte> out) const;
  void Sync();
  [[nodiscard]] std::size_t SizeBytes() const;

 private:
  int fd_ = -1;
};

}  // namespace minidb
