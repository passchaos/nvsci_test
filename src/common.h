#ifndef NVSCIPC_COMMON_H
#define NVSCIPC_COMMON_H

#include <nvscibuf.h>
#include <nvscisync.h>
#include <stdint.h>

// 共享数据结构
typedef struct {
  uint32_t magic;    // 校验值，确认数据有效性
  uint32_t data_len; // 数据长度
  char data[1024];   // 实际数据
} SharedData;

// NvSciBuf 配置（用于创建共享缓冲区）
static const NvSciBufAttrListElement buf_attrs[] = {
    {NvSciBufAttrKey_ElemType, NvSciValue_Enum, {.e = NvSciBufElemType_Raw}},
    {NvSciBufAttrKey_CpuAccess,
     NvSciValue_Bool,
     {.b = NvSciTrue}}, // 允许CPU访问
    {NvSciBufAttrKey_Size,
     NvSciValue_Uint64,
     {.u64 = sizeof(SharedData)}}, // 缓冲区大小
};

// NvSciSync 配置（用于同步信号量）
static const NvSciSyncAttrListElement sync_attrs[] = {
    {NvSciSyncAttrKey_Type, NvSciValue_Enum, {.e = NvSciSyncType_Signal}},
    {NvSciSyncAttrKey_CpuAccess, NvSciValue_Bool, {.b = NvSciTrue}},
};

#endif // NVSCIPC_COMMON_H
