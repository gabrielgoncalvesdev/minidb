// src/shell/main.cpp
#include <print>

#include "minidb/common/version.hpp"

int main() {
  std::println("minidb {} — REPL em construcao", minidb::Version());
  return 0;
}
