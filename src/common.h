#ifndef NVSCIPC_COMMON_H
#define NVSCIPC_COMMON_H

// #include <cuda.h>
#include <cuda_runtime_api.h>
#include <nvscibuf.h>
#include <nvscierror.h>
#include <nvsciipc.h>
#include <nvscisync.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    NvSciSyncModule sync_module;
    NvSciBufModule buf_module;

    NvSciIpcEndpoint endpoint;

    struct NvSciIpcEndpointInfo endpoint_info;

    int32_t ipc_event_fd;
} IpcWrapper;

typedef struct {
    int device_id;
    CUuuid uuid;
} CudaClientInfo;

NvSciError ipc_init(const char *endpoint_name, IpcWrapper *ipc_wrapper);
void ipc_deinit(IpcWrapper *ipc_wrapper);
NvSciError ipc_send(IpcWrapper *ipc_wrapper, const void *data, size_t data_len);
NvSciError ipc_recv_fill(IpcWrapper *ipc_wrapper, void *buf, size_t data_len);

bool setup_cuda(CudaClientInfo *cuda_info);
void *handle_sync_logic(IpcWrapper *ipc_wrapper);

// #include <nvscibuf.h>
// #include <nvscisync.h>
// #include <stdint.h>

// // 共享数据结构
// typedef struct {
//   uint32_t magic;    // 校验值，确认数据有效性
//   uint32_t data_len; // 数据长度
//   char data[1024];   // 实际数据
// } SharedData;

// NvSciBufAttrList
// // NvSciBuf 配置（用于创建共享缓冲区）
// static const NvSciBufAttrListElement buf_attrs[] = {
//     {NvSciBufAttrKey_ElemType, NvSciValue_Enum, {.e = NvSciBufElemType_Raw}},
//     {NvSciBufAttrKey_CpuAccess,
//      NvSciValue_Bool,
//      {.b = NvSciTrue}}, // 允许CPU访问
//     {NvSciBufAttrKey_Size,
//      NvSciValue_Uint64,
//      {.u64 = sizeof(SharedData)}}, // 缓冲区大小
// };

// // NvSciSync 配置（用于同步信号量）
// static const NvSciSyncAttrListElement sync_attrs[] = {
//     {NvSciSyncAttrKey_Type, NvSciValue_Enum, {.e = NvSciSyncType_Signal}},
//     {NvSciSyncAttrKey_CpuAccess, NvSciValue_Bool, {.b = NvSciTrue}},
// };

#endif // NVSCIPC_COMMON_H
