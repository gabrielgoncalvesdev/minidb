# src/CMakeLists.txt
add_library(minidb_core
  common/version.cpp
  storage/disk/file_handle.cpp
  storage/disk/disk_manager.cpp
  buffer/lru_k_replacer.cpp
  buffer/buffer_pool_manager.cpp
)
target_include_directories(minidb_core PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(minidb_core PRIVATE minidb_warnings)

add_executable(minidb shell/main.cpp)
target_link_libraries(minidb PRIVATE minidb_core minidb_warnings)