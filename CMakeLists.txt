cmake_minimum_required(VERSION 3.30)
project(nvsci_test LANGUAGES C CUDA)

message("CUDA Language Check Result: ${CMAKE_CUDA_COMPILER_ID}")

set(CMAKE_CUDA_ARCHITECTURES native)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # 生成 compile_commands.json

set(CMAKE_LIBRARY_PATH /usr/lib/aarch64-linux-gnu/tegra)

find_path(NVSCICOMMON_INCLUDE_DIR
  NAMES nvscibuf.h nvscisync.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(NOT NVSCICOMMON_INCLUDE_DIR)
    message(FATAL_ERROR "NvSci headers not found! Install JetPack first.")
endif()

# find_library(CUDA_LIBRARY
#   NAMES cuda
#   PATHS /usr/lib/aarch64-linux-gnu/tegra
# )
find_library(NVSCIBUF_LIB
  NAMES nvscibuf
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)
find_library(NVSCISYNC_LIB
  NAMES nvscisync
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)
find_library(NVSCIEVENT_LIB
  NAMES nvscievent
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)
find_library(NVSCIIPC_LIB
  NAMES nvsciipc
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)
find_library(NVSCISTREAM_LIB
  NAMES nvscistream
  PATHS /usr/lib/aarch64-linux-gnu/tegra
)

if(NOT NVSCIBUF_LIB OR NOT NVSCISYNC_LIB)
    message(FATAL_ERROR "NvSci libraries not found! Install JetPack first.")
endif()

add_executable(sender src/sender.cu src/common.cu src/cuda.cu)
add_executable(receiver src/receiver.cu src/common.cu src/cuda.cu)

target_include_directories(sender PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NVSCICOMMON_INCLUDE_DIR}
    /usr/local/cuda/targets/aarch64-linux/include)
target_include_directories(receiver PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NVSCICOMMON_INCLUDE_DIR}
    /usr/local/cuda/targets/aarch64-linux/include)

target_link_libraries(sender ${NVSCIBUF_LIB}
    ${NVSCISYNC_LIB} ${NVSCIEVENT_LIB} ${NVSCIIPC_LIB} ${NVSCISTREAM_LIB}
    cuda
    )
target_link_libraries(receiver
    ${NVSCIBUF_LIB} ${NVSCISYNC_LIB} ${NVSCIEVENT_LIB} ${NVSCIIPC_LIB} ${NVSCISTREAM_LIB}
    cuda
    )
