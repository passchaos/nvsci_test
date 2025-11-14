#ifndef CU_COMMON_H
#define CU_COMMON_H

#include <cuda_runtime_api.h>

typedef struct {
    int device_id;
    CUuuid uuid;
} CudaClientInfo;

bool setup_cuda(CudaClientInfo *cuda_info);

#endif
