# CMakeLists.txt  (na raiz do projeto)
  cmake_minimum_required(VERSION 3.25)          # 3.25+ = presets v6 e FetchContent SYSTEM
  project(minidb LANGUAGES CXX VERSION 0.1.0)

  # --- Padrão da linguagem: C++23 estrito (sem extensões GNU) ---
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)           # erro se o compilador não suportar
  set(CMAKE_CXX_EXTENSIONS OFF)                
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)        


  option(MINIDB_SANITIZE "Compilar com AddressSanitizer + UBSan" OFF)
  if(MINIDB_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
  endif()


  add_library(minidb_warnings INTERFACE)
  target_compile_options(minidb_warnings INTERFACE
    -Wall -Wextra -Wpedantic -Werror
    -Wshadow -Wnon-virtual-dtor -Woverloaded-virtual
    -Wconversion -Wsign-conversion)             # pega o clássico signed/unsigned (trap p/dev Rust)

  include(cmake/FetchGoogleTest.cmake)         

  enable_testing()                              
  add_subdirectory(src)
  add_subdirectory(test)
