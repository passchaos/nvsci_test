#include "common.h"
#include <cstdio>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define CHECK_NVSCISTATUS(status, msg)                                         \
    if (status != NvSciError_Success) {                                        \
        fprintf(stderr, "Error: %s (code: %d)\n", msg, status);                \
        exit(EXIT_FAILURE);                                                    \
    }

// void handle_sync_logic(IpcWrapper *ipc_wrapper) {

// }

void *consumer(IpcWrapper *ipc_wrapper) {
    NvSciError sci_err;
    int cuda_err;

    NvSciSyncAttrList consumer_signal_attrs;
    NvSciSyncAttrList consumer_wait_attrs;

    size_t send_wait_attr_list_size = 0U;
    void *send_wait_list_desc = NULL;

    CudaClientInfo cuda_info;

    if (!setup_cuda(&cuda_info)) {
        fprintf(stderr, "Error: consumer setup_cuda failed\n");
        goto done;
    }

    sci_err = NvSciSyncAttrListCreate(ipc_wrapper->sync_module,
                                      &consumer_signal_attrs);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr,
                "consumer Error: NvSciSyncAttrListCreate failed (code: %d)\n",
                sci_err);
        goto done;
    }
    cuda_err = cudaDeviceGetNvSciSyncAttributes(
        consumer_signal_attrs, cuda_info.device_id, cudaNvSciSyncAttrSignal);

    if (cuda_err != cudaSuccess) {
        fprintf(
            stderr,
            "consumer Error: cudaDeviceGetNvSciSyncAttributes signal failed "
            "(code: %d)\n",
            cuda_err);
        goto done;
    }

    sci_err =
        NvSciSyncAttrListCreate(ipc_wrapper->sync_module, &consumer_wait_attrs);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr,
                "consumer Error: NvSciSyncAttrListCreate failed (code: %d)\n",
                sci_err);
        goto done;
    }

    cuda_err = cudaDeviceGetNvSciSyncAttributes(
        consumer_wait_attrs, cuda_info.device_id, cudaNvSciSyncAttrWait);
    if (cuda_err != cudaSuccess) {
        fprintf(stderr,
                "consumer Error: cudaDeviceGetNvSciSyncAttributes wait failed "
                "(code: %d)\n",
                cuda_err);
        goto done;
    }

    sci_err = NvSciSyncAttrListIpcExportUnreconciled(
        &consumer_wait_attrs, 1, ipc_wrapper->endpoint, &send_wait_list_desc,
        &send_wait_attr_list_size);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr,
                "consumer Error: NvSciSyncAttrListIpcExportUnreconciled failed "
                "(code: %d)\n",
                sci_err);
        goto done;
    }

    printf("begin send send_wait_attr_list_size info\n");
    sci_err = ipc_send(ipc_wrapper, &send_wait_attr_list_size,
                       sizeof(send_wait_attr_list_size));
    if (sci_err != NvSciError_Success) {
        fprintf(stderr,
                "consumer Error: ipc_send failed "
                "(code: %d)\n",
                sci_err);
        goto done;
    }

    sci_err =
        ipc_send(ipc_wrapper, &send_wait_list_desc, send_wait_attr_list_size);
    if (sci_err != NvSciError_Success) {
        fprintf(stderr,
                "consumer Error: ipc_send failed "
                "(code: %d)\n",
                sci_err);
        goto done;
    }

done:
    return NULL;
}

int main(int argc, char *argv[]) {
    IpcWrapper ipc_wrapper;
    NvSciError err;

    char *endpoint_name = argv[1];
    printf("receiver endpoint name: %s\n", endpoint_name);

    err = ipc_init(endpoint_name, &ipc_wrapper);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_init failed (code: %d)\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("begin send handshake\n");
    // const int send_handshake = 123444;
    // err = ipc_send(&ipc_wrapper, &send_handshake, sizeof(send_handshake));
    // printf("finish send handshake!\n");
    // if (err != NvSciError_Success) {
    //     fprintf(stderr, "Error: ipc_send failed (code: %d)\n", err);

    //     ipc_deinit(&ipc_wrapper);
    //     exit(EXIT_FAILURE);
    // }

    // sleep(1000);
    // int recv_handshake = 0;
    // err = ipc_recv_fill(&ipc_wrapper, &recv_handshake,
    // sizeof(recv_handshake)); if (err != NvSciError_Success) {
    //     fprintf(stderr, "Error: ipc_recv_fill failed (code: %d)\n", err);

    //     ipc_deinit(&ipc_wrapper);
    //     exit(EXIT_FAILURE);
    // }

    // printf("recv handshake info: %d\n", recv_handshake);
    // sleep(1000);
    consumer(&ipc_wrapper);

    return 0;
}
