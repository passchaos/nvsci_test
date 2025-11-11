#include "common.h"
#include <cuda.h>
#include <stdbool.h>

bool setup_cuda(CudaClientInfo *cuda_info) {
    cudaError_t cuda_err;
    CUresult cuda_driver_err;

    cuda_info->device_id = 0;

    int num_of_gpus = 0;
    cuda_err = cudaGetDeviceCount(&num_of_gpus);
    if (cuda_err != cudaSuccess) {
        fprintf(stderr, "Failed to get number of GPUs: %s\n",
                cudaGetErrorString(cuda_err));
        return false;
    }

    cuda_err = cudaSetDevice(cuda_info->device_id);
    if (cuda_err != cudaSuccess) {
        fprintf(stderr, "Failed to set device: %s\n",
                cudaGetErrorString(cuda_err));
        return false;
    }

    cuda_driver_err =
        cuDeviceGetUuid_v2(&cuda_info->uuid, cuda_info->device_id);
    if (cuda_driver_err != CUDA_SUCCESS) {
        fprintf(stderr, "Failed to get device UUID: %s\n",
                cudaGetErrorString(cuda_err));
        return false;
    }

    return true;
}
