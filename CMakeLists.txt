cmake_minimum_required(VERSION 3.30)
project(nvsci_test C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)   # 强制要求标准，不 fallback 到低版本
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 生成 compile_commands.json


find_path(NVSCICOMMON_INCLUDE_DIR
  NAMES nvscibuf.h nvscisync.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(NOT NVSCICOMMON_INCLUDE_DIR)
  message(FATAL_ERROR "NvSci headers not found! Install JetPack first.")
endif()

find_library(NVSCIBUF_LIB
  NAMES nvscibuf
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)
find_library(NVSCISYNC_LIB
  NAMES nvscisync
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)
if(NOT NVSCIBUF_LIB OR NOT NVSCISYNC_LIB)
  message(FATAL_ERROR "NvSci libraries not found! Install JetPack first.")
endif()

add_executable(sender src/sender.c)
add_executable(receiver src/receiver.c)

target_include_directories(sender PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${NVSCICOMMON_INCLUDE_DIR})
target_include_directories(receiver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${NVSCICOMMON_INCLUDE_DIR})

target_link_libraries(sender ${NVSCIBUF_LIB} ${NVSCISYNC_LIB})
target_link_libraries(receiver ${NVSCIBUF_LIB} ${NVSCISYNC_LIB})
