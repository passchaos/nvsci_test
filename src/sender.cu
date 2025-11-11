#include "common.h"
#include <nvscibuf.h>
#include <nvscisync.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 错误处理宏
#define CHECK_NVSCISTATUS(status, msg)                                         \
    if (status != NvSciError_Success) {                                        \
        fprintf(stderr, "Error: %s (code: %d)\n", msg, status);                \
        exit(EXIT_FAILURE);                                                    \
    }

void *producer(IpcWrapper *ipc_wrapper) {
    NvSciError sci_err;
    int cuda_err;
    NvSciSyncAttrList producer_signal_attrs;
    NvSciSyncAttrList producer_wait_attrs;

    CudaClientInfo cuda_info;

    size_t send_wait_attr_list_size = 0U;
    void *send_wait_list_desc = NULL;

    if (!setup_cuda(&cuda_info)) {
        fprintf(stderr, "Error: setup_cuda failed\n");
        goto done;
    }

    sci_err = NvSciSyncAttrListCreate(ipc_wrapper->sync_module,
                                      &producer_signal_attrs);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListCreate failed (code: %d)\n",
                sci_err);
        goto done;
    }

    cuda_err = cudaDeviceGetNvSciSyncAttributes(
        producer_signal_attrs, cuda_info.device_id, cudaNvSciSyncAttrSignal);
    if (cuda_err != cudaSuccess) {
        fprintf(stderr,
                "Error: cudaDeviceGetNvSciSyncAttributes failed (code: %d)\n",
                cuda_err);
        goto done;
    }

    fprintf(stderr, "cudaDeviceGetNvSciSyncAttributes succeeded\n");

    sci_err =
        NvSciSyncAttrListCreate(ipc_wrapper->sync_module, &producer_wait_attrs);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListCreate failed (code: %d)\n",
                sci_err);
        goto done;
    }

    cuda_err = cudaDeviceGetNvSciSyncAttributes(
        producer_wait_attrs, cuda_info.device_id, cudaNvSciSyncAttrSignal);
    if (cuda_err != cudaSuccess) {
        fprintf(stderr,
                "Error: cudaDeviceGetNvSciSyncAttributes failed (code: %d)\n",
                cuda_err);
        goto done;
    }
    fprintf(
        stderr,
        "cudaDeviceGetNvSciSyncAttributes for producer_wait_attrs succeeded\n");

    sci_err = NvSciSyncAttrListIpcExportUnreconciled(
        &producer_wait_attrs, 1, ipc_wrapper->endpoint, &send_wait_list_desc,
        &send_wait_attr_list_size);
    if (sci_err != NvSciError_Success) {
        fprintf(
            stderr,
            "Error: NvSciSyncAttrListIpcExportUnreconciled failed (code: %d)\n",
            sci_err);
        goto done;
    }

    sci_err = ipc_send(ipc_wrapper, &send_wait_attr_list_size,
                       sizeof(send_wait_attr_list_size));
    if (sci_err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed (code: %d)\n", sci_err);
        goto done;
    }

    sci_err =
        ipc_send(ipc_wrapper, &send_wait_list_desc, send_wait_attr_list_size);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed (code: %d)\n", sci_err);
        goto done;
    }

// Initialize producer_signal_attrs here if needed
done:
    return NULL;
}

int main() {
    IpcWrapper ipc_wrapper;
    NvSciError err;

    err = ipc_init("nvscisync_a_0", &ipc_wrapper);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_init failed (code: %d)\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    // uint32_t event = 8u;
    // uint32_t value = 0x02u;
    // printf("event: %d value: %d and: %d\n", event, value, event & value);
    // return 0;

    printf("begin send handshake\n");
    const int send_handshake = 123444;
    err = ipc_send(&ipc_wrapper, &send_handshake, sizeof(send_handshake));
    printf("finish send handshake!\n");
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed (code: %d)\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    return 0;
}
