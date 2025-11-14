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

#ifdef __cplusplus
extern "C" {
#endif

NvSciError ipc_init(const char *endpoint_name, IpcWrapper *ipc_wrapper);
void ipc_deinit(IpcWrapper *ipc_wrapper);
NvSciError ipc_send(IpcWrapper *ipc_wrapper, const void *data, size_t data_len);
NvSciError ipc_recv_fill(IpcWrapper *ipc_wrapper, void *buf, size_t data_len);

bool setup_cuda(CudaClientInfo *cuda_info);
void *handle_sync_logic(IpcWrapper *ipc_wrapper);

#ifdef __cplusplus
}
#endif

#endif // NVSCIPC_COMMON_H
