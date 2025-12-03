#include "SX1278.h"
#include "main.h"   // 获取引脚宏 (LORA_NSS_Pin 等)
#include "rtthread.h"

/* 引入 SPI 句柄 (CubeMX 生成的在 main.c 里，这里需要 extern) */
extern SPI_HandleTypeDef hspi1; 

/* 定义驱动结构体变量 */
SX1278_hw_t lora_hw;
SX1278_t lora;

/* LoRa 初始化函数 */
int LoRa_Init(void)
{
    rt_kprintf("Starting LoRa Init...\n");

    /* 1. 配置硬件抽象层 (映射引脚) */
    /* 注意：这些宏 (LORA_DIO0_Pin 等) 必须在 main.h 里定义过 */
    /* 如果报错，请检查 CubeMX 里引脚 User Label 是否填对 */
    lora_hw.dio0.port  = LORA_DIO0_GPIO_Port;
    lora_hw.dio0.pin   = LORA_DIO0_Pin;
    
    lora_hw.nss.port   = LORA_NSS_GPIO_Port;
    lora_hw.nss.pin    = LORA_NSS_Pin;
    
    lora_hw.reset.port = LORA_RST_GPIO_Port;
    lora_hw.reset.pin  = LORA_RST_Pin;
    
    lora_hw.spi        = &hspi1; // 你的 SPI 句柄

    /* 2. 将硬件层挂载到逻辑层 */
    lora.hw = &lora_hw;

    /* 3. 初始化模块 */
    /* 参数依据你的论文设计：433MHz, 17dBm, SF9, BW125kHz */
    /*[cite: 1]: 论文中提到 SF9, BW125k, 433MHz */
    SX1278_init(&lora, 433000000, SX1278_POWER_17DBM, SX1278_LORA_SF_9,
                SX1278_LORA_BW_125KHZ, SX1278_LORA_CR_4_5, SX1278_LORA_CRC_EN, 10);

    /* 4. 验证是否成功 (读取版本号) */
    // 0x42 是 Version 寄存器地址，SX1278 的默认值通常是 0x12
    uint8_t version = SX1278_SPIRead(&lora, 0x42);
    
    rt_kprintf("LoRa Check: RegVersion = 0x%02X (Expect 0x12)\n", version);

    if (version == 0x12) {
        return RT_EOK;
    } else {
        rt_kprintf("LoRa Error: Hardware connection failed!\n");
        return -RT_ERROR;
    }
}




