#include "minidb/storage/disk/file_handle.hpp"

#include <fcntl.h>     // ::open, O_RDWR, O_CREAT
#include <sys/stat.h>  // ::fstat, struct stat
#include <unistd.h>    // ::pread, ::pwrite, ::close, ::fsync

#include <cerrno>
#include <cstring>     // std::strerror
#include <format>
#include <stdexcept>   // std::runtime_error
#include <utility>     // std::exchange

namespace minidb {

FileHandle::FileHandle(const std::string& path) {
  fd_ = ::open(path.c_str(), O_RDWR | O_CREAT, 0644);  // 0644 = rw-r--r--
  if (fd_ < 0) {
    throw std::runtime_error(
        std::format("FileHandle: falha ao abrir '{}': {}", path, std::strerror(errno)));
  }
}

FileHandle::~FileHandle() {
  if (fd_ >= 0) {
    ::close(fd_);  // erro de close no dtor: não há o que fazer, ignoramos
  }
}

// std::exchange(other.fd_, -1): devolve o valor antigo e escreve -1 em other.
FileHandle::FileHandle(FileHandle&& other) noexcept
    : fd_(std::exchange(other.fd_, -1)) {}

FileHandle& FileHandle::operator=(FileHandle&& other) noexcept {
  if (this != &other) {           // proteção contra auto-move
    if (fd_ >= 0) {
      ::close(fd_);               // fecha o nosso antes de sobrescrever
    }
    fd_ = std::exchange(other.fd_, -1);
  }
  return *this;
}

bool FileHandle::WriteAt(std::size_t offset, std::span<const std::byte> data) {
  std::size_t written = 0;
  while (written < data.size()) {  // pwrite pode escrever parcial: repetimos
    const ssize_t n = ::pwrite(fd_, data.data() + written, data.size() - written,
                               static_cast<off_t>(offset + written));
    if (n <= 0) {
      return false;
    }
    written += static_cast<std::size_t>(n);
  }
  return true;
}

std::size_t FileHandle::ReadAt(std::size_t offset, std::span<std::byte> out) const {
  std::size_t total = 0;
  while (total < out.size()) {
    const ssize_t n = ::pread(fd_, out.data() + total, out.size() - total,
                              static_cast<off_t>(offset + total));
    if (n < 0) {
      return total;  // erro
    }
    if (n == 0) {
      break;         // fim do arquivo
    }
    total += static_cast<std::size_t>(n);
  }
  return total; 
}

void FileHandle::Sync() {
  if (fd_ >= 0) {
    ::fsync(fd_);
  }
}

std::size_t FileHandle::SizeBytes() const {
  struct stat st{};
  if (::fstat(fd_, &st) != 0) {
    return 0;
  }
  return static_cast<std::size_t>(st.st_size);
}

}  