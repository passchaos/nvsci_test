#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_NVSCISTATUS(status, msg)                                         \
  if (status != NvSciError_Success) {                                          \
    fprintf(stderr, "Error: %s (code: %d)\n", msg, status);                    \
    exit(EXIT_FAILURE);                                                        \
  }

int main() {
  NvSciError status;
  NvSciBufAttrList receiver_buf_attrs = NULL;
  NvSciBufAttrList sender_buf_attrs = NULL;
  NvSciBufAttrList merged_buf_attrs = NULL;
  NvSciBufObj buf_obj = NULL;
  NvSciSyncAttrList receiver_sync_attrs = NULL;
  NvSciSyncAttrList sender_sync_attrs = NULL;
  NvSciSyncAttrList merged_sync_attrs = NULL;
  NvSciSyncObj sync_obj = NULL;
  SharedData *shared_data = NULL;

  // 1. 初始化 NvSciBuf 和 NvSciSync
  status = NvSciBufInit();
  CHECK_NVSCISTATUS(status, "NvSciBufInit failed");
  status = NvSciSyncInit();
  CHECK_NVSCISTATUS(status, "NvSciSyncInit failed");

  // 2. 创建缓冲区属性并保存到文件（供发送进程读取）
  status = NvSciBufAttrListCreate(&receiver_buf_attrs);
  CHECK_NVSCISTATUS(status, "Create receiver buf attrs failed");
  status = NvSciBufAttrListSetAttrs(receiver_buf_attrs, buf_attrs,
                                    sizeof(buf_attrs) / sizeof(buf_attrs[0]));
  CHECK_NVSCISTATUS(status, "Set receiver buf attrs failed");
  status =
      NvSciBufAttrListSaveToFile(receiver_buf_attrs, "receiver_buf_attrs.bin");
  CHECK_NVSCISTATUS(status, "Save receiver buf attrs failed");

  // 等待发送进程创建并保存属性文件（实际应用中需更可靠的同步）
  printf("Receiver: Waiting for sender's buf attrs...\n");
  while ((status = NvSciBufAttrListLoadFromFile(
              "sender_buf_attrs.bin", &sender_buf_attrs)) != NvSciError_Success)
    ;
  CHECK_NVSCISTATUS(status, "Load sender buf attrs failed");

  // 合并缓冲区属性
  status = NvSciBufAttrListMerge(receiver_buf_attrs, sender_buf_attrs,
                                 &merged_buf_attrs);
  CHECK_NVSCISTATUS(status, "Merge buf attrs failed");

  // 3. 打开共享缓冲区（与发送进程共享同一块内存）
  status = NvSciBufObjOpen(merged_buf_attrs, &buf_obj);
  CHECK_NVSCISTATUS(status, "Open buf obj failed");

  // 映射缓冲区到本地地址空间
  status = NvSciBufObjGetCpuPtr(buf_obj, (void **)&shared_data);
  CHECK_NVSCISTATUS(status, "Get cpu ptr failed");

  // 4. 创建同步属性并保存到文件
  status = NvSciSyncAttrListCreate(&receiver_sync_attrs);
  CHECK_NVSCISTATUS(status, "Create receiver sync attrs failed");
  status =
      NvSciSyncAttrListSetAttrs(receiver_sync_attrs, sync_attrs,
                                sizeof(sync_attrs) / sizeof(sync_attrs[0]));
  CHECK_NVSCISTATUS(status, "Set receiver sync attrs failed");
  status = NvSciSyncAttrListSaveToFile(receiver_sync_attrs,
                                       "receiver_sync_attrs.bin");
  CHECK_NVSCISTATUS(status, "Save receiver sync attrs failed");

  // 等待发送进程的同步属性
  printf("Receiver: Waiting for sender's sync attrs...\n");
  while ((status = NvSciSyncAttrListLoadFromFile("sender_sync_attrs.bin",
                                                 &sender_sync_attrs)) !=
         NvSciError_Success)
    ;
  CHECK_NVSCISTATUS(status, "Load sender sync attrs failed");

  // 合并同步属性
  status = NvSciSyncAttrListMerge(receiver_sync_attrs, sender_sync_attrs,
                                  &merged_sync_attrs);
  CHECK_NVSCISTATUS(status, "Merge sync attrs failed");

  // 打开同步对象
  status = NvSciSyncObjOpen(merged_sync_attrs, &sync_obj);
  CHECK_NVSCISTATUS(status, "Open sync obj failed");

  // 5. 等待发送进程的信号（信号量达到1）
  printf("Receiver: Waiting for data...\n");
  status = NvSciSyncObjWait(sync_obj, 1, UINT64_MAX);
  CHECK_NVSCISTATUS(status, "Wait for sender failed");

  // 读取并验证数据
  if (shared_data->magic != 0xDEADBEEF) {
    fprintf(stderr, "Receiver: Data invalid!\n");
    exit(EXIT_FAILURE);
  }
  printf("Receiver: Received data: %s (len: %u)\n", shared_data->data,
         shared_data->data_len);

  // 6. 发送确认信号（信号量+1至2）
  status = NvSciSyncObjSignal(sync_obj, 2);
  CHECK_NVSCISTATUS(status, "Signal ack failed");
  printf("Receiver: Sent ack to sender\n");

  // 7. 释放资源
  NvSciBufObjFree(buf_obj);
  NvSciSyncObjFree(sync_obj);
  NvSciBufAttrListDestroy(receiver_buf_attrs);
  NvSciBufAttrListDestroy(sender_buf_attrs);
  NvSciBufAttrListDestroy(merged_buf_attrs);
  NvSciSyncAttrListDestroy(receiver_sync_attrs);
  NvSciSyncAttrListDestroy(sender_sync_attrs);
  NvSciSyncAttrListDestroy(merged_sync_attrs);
  NvSciBufDeinit();
  NvSciSyncDeinit();

  return 0;
}
