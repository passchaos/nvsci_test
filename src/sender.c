#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 错误处理宏
#define CHECK_NVSCISTATUS(status, msg)                                         \
  if (status != NvSciError_Success) {                                          \
    fprintf(stderr, "Error: %s (code: %d)\n", msg, status);                    \
    exit(EXIT_FAILURE);                                                        \
  }

int main() {
  NvSciError status;
  NvSciBufAttrList sender_buf_attrs = NULL;
  NvSciBufAttrList receiver_buf_attrs = NULL;
  NvSciBufAttrList merged_buf_attrs = NULL;
  NvSciBufObj buf_obj = NULL;
  NvSciSyncAttrList sender_sync_attrs = NULL;
  NvSciSyncAttrList receiver_sync_attrs = NULL;
  NvSciSyncAttrList merged_sync_attrs = NULL;
  NvSciSyncObj sync_obj = NULL;
  SharedData *shared_data = NULL;

  // 1. 初始化 NvSciBuf 和 NvSciSync
  status = NvSciBufInit();
  CHECK_NVSCISTATUS(status, "NvSciBufInit failed");
  status = NvSciSyncInit();
  CHECK_NVSCISTATUS(status, "NvSciSyncInit failed");

  // 2. 创建缓冲区属性列表并协商
  status = NvSciBufAttrListCreate(&sender_buf_attrs);
  CHECK_NVSCISTATUS(status, "Create sender buf attrs failed");
  status = NvSciBufAttrListSetAttrs(sender_buf_attrs, buf_attrs,
                                    sizeof(buf_attrs) / sizeof(buf_attrs[0]));
  CHECK_NVSCISTATUS(status, "Set sender buf attrs failed");

  // 从文件加载接收进程的缓冲区属性（实际应用中需通过IPC传递）
  status = NvSciBufAttrListLoadFromFile("receiver_buf_attrs.bin",
                                        &receiver_buf_attrs);
  CHECK_NVSCISTATUS(status, "Load receiver buf attrs failed");

  // 合并属性列表（协商双方都支持的配置）
  status = NvSciBufAttrListMerge(sender_buf_attrs, receiver_buf_attrs,
                                 &merged_buf_attrs);
  CHECK_NVSCISTATUS(status, "Merge buf attrs failed");

  // 3. 创建共享缓冲区
  status = NvSciBufObjCreate(merged_buf_attrs, &buf_obj);
  CHECK_NVSCISTATUS(status, "Create buf obj failed");

  // 映射缓冲区到本地进程地址空间
  status = NvSciBufObjGetCpuPtr(buf_obj, (void **)&shared_data);
  CHECK_NVSCISTATUS(status, "Get cpu ptr failed");

  // 4. 创建同步对象（信号量）
  status = NvSciSyncAttrListCreate(&sender_sync_attrs);
  CHECK_NVSCISTATUS(status, "Create sender sync attrs failed");
  status =
      NvSciSyncAttrListSetAttrs(sender_sync_attrs, sync_attrs,
                                sizeof(sync_attrs) / sizeof(sync_attrs[0]));
  CHECK_NVSCISTATUS(status, "Set sender sync attrs failed");

  // 加载接收进程的同步属性
  status = NvSciSyncAttrListLoadFromFile("receiver_sync_attrs.bin",
                                         &receiver_sync_attrs);
  CHECK_NVSCISTATUS(status, "Load receiver sync attrs failed");

  // 合并同步属性
  status = NvSciSyncAttrListMerge(sender_sync_attrs, receiver_sync_attrs,
                                  &merged_sync_attrs);
  CHECK_NVSCISTATUS(status, "Merge sync attrs failed");

  // 创建同步对象
  status = NvSciSyncObjCreate(merged_sync_attrs, &sync_obj);
  CHECK_NVSCISTATUS(status, "Create sync obj failed");

  // 5. 写入数据到共享缓冲区
  shared_data->magic = 0xDEADBEEF;
  strcpy(shared_data->data, "Hello from NvSci sender!");
  shared_data->data_len = strlen(shared_data->data);
  printf("Sender: Wrote data: %s\n", shared_data->data);

  // 6. 发送信号通知接收进程（信号量+1）
  status = NvSciSyncObjSignal(sync_obj, 1);
  CHECK_NVSCISTATUS(status, "Signal sync obj failed");
  printf("Sender: Notified receiver\n");

  // 等待接收进程确认（实际应用中可循环通信）
  printf("Sender: Waiting for receiver...\n");
  status = NvSciSyncObjWait(sync_obj, 2, UINT64_MAX); // 等待信号量达到2
  CHECK_NVSCISTATUS(status, "Wait for receiver failed");
  printf("Sender: Received ack from receiver\n");

  // 7. 释放资源
  NvSciBufObjFree(buf_obj);
  NvSciSyncObjFree(sync_obj);
  NvSciBufAttrListDestroy(sender_buf_attrs);
  NvSciBufAttrListDestroy(receiver_buf_attrs);
  NvSciBufAttrListDestroy(merged_buf_attrs);
  NvSciSyncAttrListDestroy(sender_sync_attrs);
  NvSciSyncAttrListDestroy(receiver_sync_attrs);
  NvSciSyncAttrListDestroy(merged_sync_attrs);
  NvSciBufDeinit();
  NvSciSyncDeinit();

  return 0;
}
