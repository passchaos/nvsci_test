cmake_minimum_required(VERSION 3.30)
project(nvsci_test LANGUAGES C CUDA)

set(CMAKE_CUDA_ARCHITECTURES native)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_LIBRARY_PATH /usr/lib/aarch64-linux-gnu/tegra)

find_path(NVSCICOMMON_INCLUDE_DIR
  NAMES nvscibuf.h nvscisync.h
  PATHS ${CMAKE_CURRENT_SOURCE_DIR}/include
)
if(NOT NVSCICOMMON_INCLUDE_DIR)
    message(FATAL_ERROR "NvSci headers not found! Install JetPack first.")
endif()

find_library(NVSCIBUF_LIB
  NAMES nvscibuf
)
find_library(NVSCISYNC_LIB
  NAMES nvscisync
)
find_library(NVSCIEVENT_LIB
  NAMES nvscievent
)
find_library(NVSCIIPC_LIB
  NAMES nvsciipc
)
find_library(NVSCISTREAM_LIB
  NAMES nvscistream
)

if(NOT NVSCIBUF_LIB OR NOT NVSCISYNC_LIB)
    message(FATAL_ERROR "NvSci libraries not found! Install JetPack first.")
endif()

add_library(common_config INTERFACE)
target_include_directories(common_config INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NVSCICOMMON_INCLUDE_DIR}
    /usr/local/cuda/targets/aarch64-linux/include)
target_link_libraries(common_config INTERFACE
    ${NVSCIBUF_LIB}
    ${NVSCISYNC_LIB}
    ${NVSCIEVENT_LIB}
    ${NVSCIIPC_LIB}
    ${NVSCISTREAM_LIB}
    cuda
)

add_executable(sender src/sender.cu src/common.c src/cuda.cu)
add_executable(receiver src/receiver.cu src/common.c src/cuda.cu)

add_executable(cpu_sender src/cpu_sender.c src/common.c)
# set_target_properties(cpu_sender PROPERTIES LINKER_LANGUAGE CXX)
add_executable(cpu_receiver src/cpu_receiver.c src/common.c)
# set_target_properties(cpu_receiver PROPERTIES LINKER_LANGUAGE CXX)

# target_include_directories(sender PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#     ${NVSCICOMMON_INCLUDE_DIR}
#     /usr/local/cuda/targets/aarch64-linux/include)
# target_include_directories(receiver PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}/include
#     ${NVSCICOMMON_INCLUDE_DIR}
#     /usr/local/cuda/targets/aarch64-linux/include)

target_link_libraries(sender PRIVATE common_config
    )
target_link_libraries(receiver PRIVATE common_config
    )
target_link_libraries(cpu_sender PRIVATE common_config
    )
target_link_libraries(cpu_receiver PRIVATE common_config
    )
