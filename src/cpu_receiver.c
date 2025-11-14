#include "common.h"

int main() {
    IpcWrapper ipc_wrapper;
    NvSciError err;

    err = ipc_init("nvscisync_a_1", &ipc_wrapper);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc init failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncAttrList waiterAttrList;
    err = NvSciSyncAttrListCreate(ipc_wrapper.sync_module, &waiterAttrList);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListCreate failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncAttrKeyValuePair keyValue[2] = {};
    bool cpuSignaler = true;
    keyValue[0].attrKey = NvSciSyncAttrKey_NeedCpuAccess;
    keyValue[0].value = (void *)&cpuSignaler;
    keyValue[0].len = sizeof(cpuSignaler);

    NvSciSyncAccessPerm cpuPerm = NvSciSyncAccessPerm_WaitOnly;
    keyValue[1].attrKey = NvSciSyncAttrKey_RequiredPerm;
    keyValue[1].value = (void *)&cpuPerm;
    keyValue[1].len = sizeof(cpuPerm);

    err = NvSciSyncAttrListSetAttrs(waiterAttrList, keyValue, 2);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListSetAttrs failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("waiting for signal...\n");

    void *waiterAttrListDesc;
    size_t waiterAttrListSize;
    err = NvSciSyncAttrListIpcExportUnreconciled(
        &waiterAttrList, 1, ipc_wrapper.endpoint, &waiterAttrListDesc,
        &waiterAttrListSize);
    if (err != NvSciError_Success) {
        fprintf(stderr,
                "Error: NvSciSyncAttrListIpcExportUnreconciled failed: %d\n",
                err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    err =
        ipc_send(&ipc_wrapper, &waiterAttrListSize, sizeof(waiterAttrListSize));
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    err = ipc_send(&ipc_wrapper, waiterAttrListDesc, waiterAttrListSize);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("send waiter attr list success\n");
    // wait to receive sync obj

    size_t recv_sync_obj_desc_size = 0;

    err = ipc_recv_fill(&ipc_wrapper, &recv_sync_obj_desc_size,
                        sizeof(recv_sync_obj_desc_size));
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_recv failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    void *recv_sync_obj_desc = malloc(recv_sync_obj_desc_size);
    err = ipc_recv_fill(&ipc_wrapper, recv_sync_obj_desc,
                        recv_sync_obj_desc_size);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_recv failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncObj receive_sync_obj;
    err = NvSciSyncIpcImportAttrListAndObj(
        ipc_wrapper.sync_module, ipc_wrapper.endpoint, recv_sync_obj_desc,
        recv_sync_obj_desc_size, &waiterAttrList, 1,
        NvSciSyncAccessPerm_WaitOnly, 10000, &receive_sync_obj);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncIpcImportAttrListAndObj failed: %d\n",
                err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncFenceIpcExportDescriptor sync_fence_desc;
    err = ipc_recv_fill(&ipc_wrapper, &sync_fence_desc,
                        sizeof(NvSciSyncFenceIpcExportDescriptor));
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_recv failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncFence sync_fence = NvSciSyncFenceInitializer;
    err = NvSciSyncIpcImportFence(receive_sync_obj, &sync_fence_desc,
                                  &sync_fence);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncIpcImportFence failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncCpuWaitContext cpu_wait_context = NULL;
    err = NvSciSyncCpuWaitContextAlloc(ipc_wrapper.sync_module,
                                       &cpu_wait_context);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncCpuWaitContextCreate failed: %d\n",
                err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("begin wait by sync fence\n");
    err = NvSciSyncFenceWait(&sync_fence, cpu_wait_context, 10 * 1000 * 1000);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncFenceWait failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("receiver finished!\n");
}
