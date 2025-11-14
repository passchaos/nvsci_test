#include "common.h"
// #include <cstdio>
#include <unistd.h>

int main() {
    IpcWrapper ipc_wrapper;
    NvSciError err;

    err = ipc_init("nvscisync_a_0", &ipc_wrapper);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc init failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    // NvSciSyncAttrList unreconciledList[2] = {NULL};
    // NvSciSyncAttrList reconciledList = NULL;
    // NvSciSyncAttrList newConflictList = NULL;
    // NvSciSyncAttrList signalerAttrList = NULL;

    // NvSciSyncObj syncObj = NULL;

    // NvSciSyncAttrList importedUnreconciledAttrList = NULL;

    // NvSciSyncFence syncFence = NvSciSyncFenceInitializer;

    // NvSciSyncFenceIpcExportDescriptor fenceDesc;

    NvSciSyncAttrKeyValuePair keyValue[2] = {};
    bool cpuSignaler = true;
    keyValue[0].attrKey = NvSciSyncAttrKey_NeedCpuAccess;
    keyValue[0].value = (void *)&cpuSignaler;
    keyValue[0].len = sizeof(cpuSignaler);

    NvSciSyncAccessPerm cpuPerm = NvSciSyncAccessPerm_SignalOnly;
    keyValue[1].attrKey = NvSciSyncAttrKey_RequiredPerm;
    keyValue[1].value = (void *)&cpuPerm;
    keyValue[1].len = sizeof(cpuPerm);

    NvSciSyncAttrList sender_signal_attr_list;
    err = NvSciSyncAttrListCreate(ipc_wrapper.sync_module,
                                  &sender_signal_attr_list);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListCreate failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    err = NvSciSyncAttrListSetAttrs(sender_signal_attr_list, keyValue, 2);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListSetAttrs failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    size_t receiver_wait_attr_list_size;
    err = ipc_recv_fill(&ipc_wrapper, &receiver_wait_attr_list_size,
                        sizeof(receiver_wait_attr_list_size));
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_recv_fill failed: %d\n", err);

        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    void *receiver_wait_attr_list_desc = malloc(receiver_wait_attr_list_size);
    if (!receiver_wait_attr_list_desc) {
        fprintf(stderr, "Error: malloc failed\n");
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    err = ipc_recv_fill(&ipc_wrapper, receiver_wait_attr_list_desc,
                        receiver_wait_attr_list_size);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_recv_fill failed: %d\n", err);

        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }
    printf("after receive receiver wait attr list desc\n");

    NvSciSyncAttrList receiver_wait_attr_list;
    err = NvSciSyncAttrListIpcImportUnreconciled(
        ipc_wrapper.sync_module, ipc_wrapper.endpoint,
        receiver_wait_attr_list_desc, receiver_wait_attr_list_size,
        &receiver_wait_attr_list);
    if (err != NvSciError_Success) {
        fprintf(stderr,
                "Error: NvSciSyncAttrListIpcImportUnreconciled failed: %d\n",
                err);

        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("begin reconcile attr lists\n");
    NvSciSyncAttrList sync_all_attrs[2], sync_conflict_attrs;
    sync_all_attrs[0] = sender_signal_attr_list;
    sync_all_attrs[1] = receiver_wait_attr_list;

    NvSciSyncAttrList sender_receiver_attr_list;
    err = NvSciSyncAttrListReconcile(
        sync_all_attrs, 2, &sender_receiver_attr_list, &sync_conflict_attrs);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncAttrListReconcile failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    printf("begin create send sync obj\n");

    // create sync obj
    NvSciSyncObj send_sync_obj;
    err = NvSciSyncObjAlloc(sender_receiver_attr_list, &send_sync_obj);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncObjAlloc failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    // send sync obj to receiver
    void *send_sync_obj_desc = NULL;
    size_t send_sync_obj_desc_size = 0;

    err = NvSciSyncIpcExportAttrListAndObj(
        send_sync_obj, NvSciSyncAccessPerm_WaitOnly, ipc_wrapper.endpoint,
        &send_sync_obj_desc, &send_sync_obj_desc_size);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncIpcExportAttrListAndObj failed: %d\n",
                err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    err = ipc_send(&ipc_wrapper, &send_sync_obj_desc_size, sizeof(size_t));
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }
    err = ipc_send(&ipc_wrapper, send_sync_obj_desc, send_sync_obj_desc_size);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncFence sync_fence = NvSciSyncFenceInitializer;
    err = NvSciSyncObjGenerateFence(send_sync_obj, &sync_fence);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncObjGenerateFence failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }
    NvSciSyncFenceIpcExportDescriptor send_fence_desc;
    err = NvSciSyncIpcExportFence(&sync_fence, ipc_wrapper.endpoint,
                                  &send_fence_desc);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncIpcExportFence failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    err = ipc_send(&ipc_wrapper, &send_fence_desc, sizeof(send_fence_desc));
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: ipc_send failed: %d\n", err);
        free(receiver_wait_attr_list_desc);
        ipc_deinit(&ipc_wrapper);
        exit(EXIT_FAILURE);
    }

    NvSciSyncFenceClear(&sync_fence);

    sleep(2);

    NvSciSyncObjSignal(send_sync_obj);
    printf("after signal send sync obj\n");

    // NvSciSyncAttrKeyValuePair keyValue[2] = {};
    // bool cpuSignaler = true;
    // keyValue[0].attrKey = NvSciSyncAttrKey_NeedCpuAccess;
    // keyValue[0].value = (void *)&cpuSignaler;
    // keyValue[0].len = sizeof(cpuSignaler);

    // NvSciSyncAccessPerm cpuPerm = NvSciSyncAccessPerm_SignalOnly;
    // keyValue[1].attrKey = NvSciSyncAttrKey_RequiredPerm;
    // keyValue[1].value = (void *)&cpuPerm;
    // keyValue[1].len = sizeof(cpuPerm);

    // NvSciSyncAttrListSetAttrs(NvSciSyncAttrList attrList, const
    // NvSciSyncAttrKeyValuePair *pairArray, size_t pairCount)

fail:
    ipc_deinit(&ipc_wrapper);
    exit(EXIT_FAILURE);
}
