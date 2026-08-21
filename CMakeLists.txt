# CMakeLists.txt  (na raiz do projeto)
  cmake_minimum_required(VERSION 3.25)          # 3.25+ = presets v6 e FetchContent SYSTEM
  project(minidb LANGUAGES CXX VERSION 0.1.0)

  # --- Padrão da linguagem: C++23 estrito (sem extensões GNU) ---
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)           # erro se o compilador não suportar
  set(CMAKE_CXX_EXTENSIONS OFF)                 # usa -std=c++23, não -std=gnu++23
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)         # gera compile_commands.json (p/ clangd)

  # --- Sanitizers: ligados pelo preset "debug" (MINIDB_SANITIZE=ON) ---
  # add_compile/link_options valem para ESTE diretório e todos os subdiretórios/alvos
  # definidos DEPOIS desta linha — inclusive o GoogleSTest, que é populado logo abaixo.
  option(MINIDB_SANITIZE "Compilar com AddressSanitizer + UBSan" OFF)
  if(MINIDB_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
  endif()

  # --- Warnings estritos, aplicados SÓ ao nosso código (não a terceiros) ---
  # INTERFACE = a lib não tem código; ela só "empresta" flags a quem a linkar.
  add_library(minidb_warnings INTERFACE)
  target_compile_options(minidb_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual
    -Wconversion -Wsign-conversion)             # pega o clássico signed/unsigned (trap p/dev Rust)

  include(cmake/FetchGoogleTest.cmake)          # baixa e configura o GoogleTest

  enable_testing()                              # habilita o ctest
  add_subdirectory(src)
  add_subdirectory(test)
