#ifndef __BSP_LORA_H__
#define __BSP_LORA_H__

#include <stdint.h>

/* 初始化函数 - 在系统启动时调用 */
int LoRa_Init(void);

/* 发送数据函数 - 供其他线程调用发送消息 */
void LoRa_Send_Data(uint8_t *data, uint8_t len);

/* 接收线程入口函数 - 供 main.c 创建线程使用 */
void lora_rx_thread_entry(void *parameter);

#endif /* __BSP_LORA_H__ */
