#include "common.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/select.h>

NvSciError ipc_init(const char *endpoint_name, IpcWrapper *ipc_wrapper) {
    NvSciError err;

    err = NvSciSyncModuleOpen(&ipc_wrapper->sync_module);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciSyncModuleOpen failed (code: %d)\n", err);

        ipc_deinit(NULL);
        exit(EXIT_FAILURE);
    }

    err = NvSciBufModuleOpen(&ipc_wrapper->buf_module);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciBufModuleOpen failed (code: %d)\n", err);

        ipc_deinit(NULL);
        exit(EXIT_FAILURE);
    }

    err = NvSciIpcInit();
    if (err != NvSciError_Success) {
        fprintf(stderr, "Error: NvSciIpcInit failed (code: %d)\n", err);

        ipc_deinit(NULL);
        exit(EXIT_FAILURE);
    }

    err = NvSciIpcOpenEndpoint(endpoint_name, &ipc_wrapper->endpoint);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Failed to open endpoint: %s\n", endpoint_name);
        return err;
    }

    err = NvSciIpcGetLinuxEventFd(ipc_wrapper->endpoint,
                                  &ipc_wrapper->ipc_event_fd);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Failed to get Linux event FD\n");
        return err;
    }

    err = NvSciIpcGetEndpointInfo(ipc_wrapper->endpoint,
                                  &ipc_wrapper->endpoint_info);
    if (err != NvSciError_Success) {
        fprintf(stderr, "Failed to get endpoint info\n");
        return err;
    }

    err = NvSciIpcResetEndpointSafe(ipc_wrapper->endpoint);

    return err;
}

void ipc_deinit(IpcWrapper *ipc_wrapper) {
    if (ipc_wrapper != NULL) {
        NvSciIpcCloseEndpoint(ipc_wrapper->endpoint);
        NvSciIpcDeinit();
    }

    if (ipc_wrapper->buf_module != NULL) {
        NvSciBufModuleClose(ipc_wrapper->buf_module);
    }

    if (ipc_wrapper->sync_module != NULL) {
        NvSciSyncModuleClose(ipc_wrapper->sync_module);
    }
}

NvSciError wait_event(IpcWrapper *ipc_wrapper, uint32_t value) {
    fd_set rfds;
    uint32_t event = 0;
    NvSciError err;

    while (true) {
        err = NvSciIpcGetEvent(ipc_wrapper->endpoint, &event);
        if (err != NvSciError_Success) {
            fprintf(stderr, "Failed to get event\n");
            return err;
        }
        printf("event: %d\n", event);
        if (0U != (event & value)) {
            printf("meet condition: %d\n", value);
            return NvSciError_Success;
        }

        FD_ZERO(&rfds);
        FD_SET(ipc_wrapper->ipc_event_fd, &rfds);

        if (select(ipc_wrapper->ipc_event_fd + 1, &rfds, NULL, NULL, NULL) <
            0) {
            return NvSciError_ResourceError;
        }

        if (!FD_ISSET(ipc_wrapper->ipc_event_fd, &rfds)) {
            return NvSciError_NvSciIpcUnknown;
        }
    }

    return NvSciError_Success;
}

NvSciError ipc_send(IpcWrapper *ipc_wrapper, const void *data,
                    size_t data_len) {
    NvSciError err;
    bool done = false;
    int32_t bytes;

    while (done == false) {
        err = wait_event(ipc_wrapper, NV_SCI_IPC_EVENT_WRITE);
        if (err != NvSciError_Success) {
            fprintf(stderr, "Failed to wait for write event\n");
            return err;
        }

        err = NvSciIpcWrite(ipc_wrapper->endpoint, data, data_len, &bytes);
        if (err != NvSciError_Success) {
            fprintf(stderr, "Failed to write data\n");
            return err;
        }
        if (bytes != data_len) {
            fprintf(stderr, "Failed to write data\n");
            return NvSciError_NvSciIpcUnknown;
        }

        done = true;
    }

    return err;
}

NvSciError ipc_recv_fill(IpcWrapper *ipc_wrapper, void *buf, size_t data_len) {
    NvSciError err = NvSciError_Success;
    bool done = false;
    int32_t bytes;

    while (done == false) {
        err = wait_event(ipc_wrapper, NV_SCI_IPC_EVENT_READ);
        if (err != NvSciError_Success) {
            fprintf(stderr, "Failed to wait for read event\n");
            return err;
        }

        err = NvSciIpcRead(ipc_wrapper->endpoint, buf, data_len, &bytes);
        if (err != NvSciError_Success) {
            fprintf(stderr, "Failed to read data\n");
            return err;
        }
        if (bytes != data_len) {
            fprintf(stderr, "Failed to read data\n");
            return NvSciError_NvSciIpcUnknown;
        }

        done = true;
    }

    return err;
}
